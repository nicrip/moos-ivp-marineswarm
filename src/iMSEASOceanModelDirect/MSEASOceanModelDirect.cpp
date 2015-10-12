/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: MSEASOceanModelDirect.cpp                       */
/*    DATE: November 2014                                   */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "MSEASOceanModelDirect.h"
#include <ctime>
#include <sstream>
#include "NodeRecordUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

MSEASOceanModelDirect::MSEASOceanModelDirect() : m_reset_time_flag(false), m_model_pos_setflag(false), m_model_varname_setflag(false), m_appcasting_ready(false)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

MSEASOceanModelDirect::~MSEASOceanModelDirect()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MSEASOceanModelDirect::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  MOOSMSG_LIST::iterator p;
  bool handled = false;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "NODE_REPORT") {
      handled = HandleNewNodeReport(msg);
      if(!handled)
        cout << "Unhandled Node Report From " << msg.GetCommunity() << endl;
    } else if (msg.GetKey() == m_varname_moosvar) {
      m_model_varnames_request = msg.GetString();
      m_model_varname_vec_request = parseString(m_model_varnames_request, ',');
      m_num_model_vars = m_model_varname_vec_request.size();
      Cell a(m_num_model_vars, 1);
      for (unsigned int i=0; i<m_num_model_vars; i++)
        a(i) = m_model_varname_vec_request[i];
      m_model_varname_cell_request = a;
    } else if (msg.GetKey() == m_time_request_offset_moosvar) {
      m_model_time_request_offset = msg.GetDouble();
    } else if (msg.GetKey() == m_time_offset_reset_moosvar) {
      m_reset_time_flag = true;
    }

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
   }

   return(true);
}

//---------------------------------------------------------
// Procedure: HandleNewNodeReport

bool MSEASOceanModelDirect::HandleNewNodeReport(CMOOSMsg msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();

  if (m_map_node_records.find(community_name) == m_map_node_records.end()) {
    // this auv node community is not to be handled by us
    return false;
  } else {
    // update our auv node position
    m_map_node_records[community_name] = string2NodeRecord(message_value);
    m_model_lon_request(m_map_node_idx[community_name]) = m_map_node_records[community_name].getLon();
    m_model_lat_request(m_map_node_idx[community_name]) = m_map_node_records[community_name].getLat();
    m_model_depth_request(m_map_node_idx[community_name]) = m_map_node_records[community_name].getDepth();
    //cout << community_name << " lon: " << m_map_node_records[community_name].getLon() << " lat: " << m_map_node_records[community_name].getLat() << " depth: " << m_map_node_records[community_name].getDepth() << endl;
    if (!m_model_pos_setflag) {
      m_nodes_pos_set[m_map_node_idx[community_name]] = true;
      m_model_pos_setflag = m_nodes_pos_set.count() == m_num_communities;
    }
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool MSEASOceanModelDirect::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool MSEASOceanModelDirect::Iterate()
{
  AppCastingMOOSApp::Iterate();
  m_iterations++;

  if(m_reset_time_flag) {
    m_model_time_offset = MOOSTime();
    m_time_request(0) = m_model_time_offset;
    m_reset_time_flag = false;
  }

  if(m_model_pos_setflag && m_model_varname_setflag) {
    RequestMSEASData();
    DistributeMSEASData();
    m_appcasting_ready = true;
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: RequestMSEASData

void MSEASOceanModelDirect::RequestMSEASData()
{
  m_model_time_request = MOOSTime() + m_model_time_request_offset;

  cout << "filepath requested: " << m_model_filepath << endl;
  cout << "varname requested: " << m_model_varnames_request << endl;
  cout << "utc model time requested: " << m_model_time_request << endl;
  cout << "utc begin time: " << m_model_time_offset << endl;
  cout << "seconds since start (delta): " << (m_model_time_request - m_model_time_offset) << endl;

  m_time_request(1) = m_model_time_request;
  m_time_request(5) = m_num_communities;
  m_time_return = feval("generate_sample_times", m_time_request, 1);
  m_model_times_request = m_time_return(0).matrix_value();

  m_batch_request(0) = m_model_varname_cell_request;
  m_batch_request(1) = m_model_lon_request;
  m_batch_request(2) = m_model_lat_request;
  m_batch_request(3) = m_model_depth_request;
  m_batch_request(4) = m_model_times_request;
  m_start = clock();
  m_batch_return = feval("readmseaspe_moossafir", m_batch_request, 1);
  m_end = clock();
  m_request_time = double(m_end-m_start)/CLOCKS_PER_SEC;

  cout << "time to process " << m_num_communities*m_model_varname_cell_request.length() << " requests: " << m_request_time << " seconds." << endl;

  m_batch_return_cell = m_batch_return(0).cell_value();
  int num_vars = m_batch_return_cell.cols();
  if (num_vars > 0) {
    int num_returns = m_batch_return_cell(0).length();
    m_model_values_return.resize(num_vars, num_returns);
    for (unsigned int i=0; i<num_vars; i++)
      m_model_values_return.insert(m_batch_return_cell(i).matrix_value(), i, 0);
  }

  cout << "number of returns: " << m_model_values_return.rows()*m_model_values_return.cols() << endl;
}

//---------------------------------------------------------
// Procedure: DistributeMSEASData

void MSEASOceanModelDirect::DistributeMSEASData()
{
  string community_name;
  double drift_x(0), drift_y(0), drift_z(0);
  int x_idx(-1), y_idx(-1), z_idx(-1);
  if (m_map_varname_idx.find("u") != m_map_varname_idx.end()) {
    x_idx = m_map_varname_idx["u"];
  }
  if (m_map_varname_idx.find("v") != m_map_varname_idx.end()) {
    y_idx = m_map_varname_idx["v"];
  }
  if (m_map_varname_idx.find("w") != m_map_varname_idx.end()) {
    z_idx = m_map_varname_idx["w"];
  }
  for (unsigned int i=0; i<m_num_communities; i++) {
    community_name = m_node_communities_vec[i];
    if (x_idx >= 0) {
      drift_x = m_model_values_return(x_idx, i)/100.0;
      Notify("DRIFT_X_FOR_" + toupper(community_name), drift_x);
      m_map_node_hdrifts[community_name].first = drift_x;
    }
    if (y_idx >= 0) {
      drift_y = m_model_values_return(y_idx, i)/100.0;
      Notify("DRIFT_Y_FOR_" + toupper(community_name), drift_y);
      m_map_node_hdrifts[community_name].second = drift_y;
    }
    if (z_idx >= 0) {
      drift_z = m_model_values_return(z_idx, i)/100.0;
      Notify("DRIFT_Z_FOR_" + toupper(community_name), drift_z);
    }
  }
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool MSEASOceanModelDirect::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);

      if(param == "FOO") {
        //handled
      }
      else if(param == "BAR") {
        //handled
      }
    }
  }

  m_timewarp = GetMOOSTimeWarp();

  // get path to octave files -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("OCTAVE_PATH", m_octave_path)) {
    cerr << "Path to required octave files OCTAVE_PATH not specified! Quitting..." << endl;
    return(false);
  }

  // get path to MSEAS model file -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("MSEAS_FILEPATH", m_model_filepath)) {
    cerr << "Path to MSEAS model file MSEAS_FILEPATH not specified! Quitting..." << endl;
    return(false);
  }

  // get list of node communities -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("NODE_COMMUNITIES", m_node_communities)) {
    cerr << "List of AUV node communities NODE_COMMUNITIES not specified! Quitting..." << endl;
    return(false);
  }

  // get list of MSEAS model variable names
  if (!m_MissionReader.GetConfigurationParam("MSEAS_VARNAMES", m_model_varnames_request)) {
    cerr << "Requested MSEAS model variable names MSEAS_VARNAMES not specified! Quitting..." << endl;
    return(false);
  }

  // get MSEAS time offset -> if unspecified assume 0
  if (!m_MissionReader.GetConfigurationParam("MSEAS_TIME_OFFSET", m_model_time_request_offset)) {
    cerr << "Requested MSEAS model time offset MSEAS_TIME_OFFSET not specified! Assuming 0.0 seconds offset..." << endl;
    m_model_time_request_offset = 0.0;
  }

  // get MOOS variable prefix -> if unspecified, assume no prefix
  if (!m_MissionReader.GetConfigurationParam("MOOSVAR_PREFIX", m_moosvar_prefix)) {
    cerr << "MOOS variable prefix MOOSVAR_PREFIX not specified! Assuming no prefix..." << endl;
    m_moosvar_prefix = "";
  }

  // get model time warp -> if unspecified, assume 1
  if (!m_MissionReader.GetConfigurationParam("MODEL_TIME_WARP", m_model_time_warp)) {
    cerr << "MSEAS ocean model time warp MODEL_TIME_WARP not specified! Assuming warp of 1.0..." << endl;
    m_model_time_warp = 1.0;
  }

  // parse and set MSEAS requested variable names
  m_model_varname_vec_request = parseString(m_model_varnames_request, ',');
  m_num_model_vars = m_model_varname_vec_request.size();
  Cell a(m_num_model_vars, 1);
  for (unsigned int i=0; i<m_num_model_vars; i++) {
    a(i) = m_model_varname_vec_request[i];
    m_map_varname_idx[m_model_varname_vec_request[i]] = i;
  }
  m_model_varname_cell_request = a;
  m_model_varname_setflag = true;

  // parse list of AUV node communites, and initialize the maps
  m_node_communities_vec = parseString(m_node_communities, ',');
  m_num_communities = m_node_communities_vec.size();
  for (unsigned int i=0; i<m_num_communities; i++) {
    m_map_node_records[m_node_communities_vec[i]] = NodeRecord();
    m_map_node_idx[m_node_communities_vec[i]] = i;
    m_map_node_hdrifts[m_node_communities_vec[i]].first = 0.0;
    m_map_node_hdrifts[m_node_communities_vec[i]].second = 0.0;
  }

  // initialize lon, lat and depth vectors for our AUV node communities
  m_model_lon_request.resize(m_num_communities);
  m_model_lat_request.resize(m_num_communities);
  m_model_depth_request.resize(m_num_communities);

  // initialize dynamic bitset for checking that we have initiliazed all node positions
  m_nodes_pos_set.resize(m_num_communities);

  // initialize instance of octave
  string_vector argv (4);
  argv(0) = "embedded";
  argv(1) = "-q";
  argv(2) = "--path";
  argv(3) = m_octave_path;
  octave_main (4, argv.c_str_vec (), 1);
  m_batch_request.resize(1);
  m_batch_return.resize(3);
  m_time_request.resize(6);
  m_time_return.resize(1);

  // set time offset
  m_model_time_offset = MOOSTime();
  m_time_request(0) = m_model_time_offset;

  // get MSEAS model time information
  m_batch_request(0) = m_model_filepath;
  m_batch_return = feval("mseaspe_model_time", m_batch_request, 1);
  m_model_start_time = m_batch_return(0).scalar_value();
  m_model_end_time = m_batch_return(1).scalar_value();
  m_model_duration = m_batch_return(2).scalar_value();
  m_time_request(2) = m_model_start_time;
  m_time_request(3) = m_model_end_time;
  m_time_request(4) = m_model_duration;
  m_batch_return.resize(1);

  // open and initialize MSEAS model file
  feval("readmseaspe_moossafir", m_batch_request, 1);
  m_batch_request.resize(5);

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void MSEASOceanModelDirect::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // m_Comms.Register("FOOBAR", 0);

  stringstream ss;

  ss << m_moosvar_prefix << "MSEASOCEANMODEL_VARNAME";
  m_varname_moosvar = ss.str();
  m_Comms.Register(m_varname_moosvar, 0);             // name of desired model variable
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODEL_TIME_REQUEST_OFFSET";
  m_time_request_offset_moosvar = ss.str();
  m_Comms.Register(m_time_request_offset_moosvar, 0); // offset in seconds from start of MSEAS model file
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODEL_TIME_OFFSET_RESET";
  m_time_offset_reset_moosvar = ss.str();
  m_Comms.Register(m_time_offset_reset_moosvar, 0);   // resets the offset time of this program
  ss.str("");
  ss.clear();

  Register("NODE_REPORT", 0.0);                       // use node reports to get lon/lat of each node, instead of passing each individually
}

//---------------------------------------------------------
// Procedure: buildReport

bool MSEASOceanModelDirect::buildReport()
{
  if (m_appcasting_ready) {
    m_msgs << std::fixed;
    m_msgs << "REQUEST:" << endl;
    m_msgs << "   Requested MSEAS Filepath: " << m_model_filepath << endl;
    m_msgs << "   Requested Position (First): LON: " << m_model_lon_request(0) << ", LAT: " << m_model_lat_request(0) << ", DEP: " << m_model_depth_request(0) << endl;
    m_msgs << "   Requested Position (Last): LON: " << m_model_lon_request(m_model_lon_request.length()-1) << ", LAT: " << m_model_lat_request(m_model_lat_request.length()-1) << ", DEP: " << m_model_depth_request(m_model_depth_request.length()-1) << endl;
    m_msgs << "   Requested Variables: " << m_model_varnames_request << endl;
    m_msgs << "   Requested Time Offset (seconds): " << m_model_time_request_offset << endl;
    m_msgs << "   Current Model Time (utc): " << m_model_time_request << endl;
    m_msgs << "   Model Start Time (utc): " << m_model_time_offset << endl;
    m_msgs << "   Requested Model Time (seconds): " << (m_model_time_request - m_model_time_offset) << endl;
    m_msgs << endl << "RETURN:" << endl;
    m_msgs << "   Processing Time for " << m_model_lon_request.length()*m_num_model_vars << " Requests (seconds): " << m_request_time << endl;
    m_msgs << "   Number of Returns: " << m_model_values_return.rows()*m_model_values_return.cols() << endl;
    m_msgs << "   Returned Value (First): " << m_model_values_return(0) << endl;
    m_msgs << "   Returned Value (Last): " << m_model_values_return(m_model_values_return.rows()-1) << endl;
    return(true);
  } else {
    m_msgs << "REQUEST:" << endl;
    m_msgs << "   No Request!" << endl;
    m_msgs << endl << "RETURN:" << endl;
    m_msgs << "   No Return!" << endl;
    return(true);
  }
}
