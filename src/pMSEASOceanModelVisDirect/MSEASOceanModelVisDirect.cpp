/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: MSEASOceanModelVisDirect.cpp                    */
/*    DATE: December 2014                                   */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "MSEASOceanModelVisDirect.h"
#include "XYFormatUtilsConvexGrid.h"
#include <math.h>
#include <ctime>
#include <sstream>

using namespace std;

//---------------------------------------------------------
// Constructor

MSEASOceanModelVisDirect::MSEASOceanModelVisDirect() : m_reset_time_flag(false), m_vis_paused(false), m_appcasting_ready(false)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

MSEASOceanModelVisDirect::~MSEASOceanModelVisDirect()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool MSEASOceanModelVisDirect::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if (msg.GetKey() == m_varname_moosvar) {
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
    } else if (msg.GetKey() == m_x_moosvar) {
      m_x = msg.GetDouble();
      SetupGrid();
    } else if (msg.GetKey() == m_y_moosvar) {
      m_y = msg.GetDouble();
      SetupGrid();
    } else if (msg.GetKey() == m_depth_moosvar) {
      m_depth = msg.GetDouble();
      SetupGrid();
    } else if (msg.GetKey() == m_color_moosvar) {
      m_grid_color_var = msg.GetString();
    } else if (msg.GetKey() == m_paused_moosvar) {
      // set as toggle for now
      if (m_vis_paused) {
        m_vis_paused = false;
      } else {
        m_vis_paused = true;
      }
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
// Procedure: OnConnectToServer

bool MSEASOceanModelVisDirect::OnConnectToServer()
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

bool MSEASOceanModelVisDirect::Iterate()
{
  AppCastingMOOSApp::Iterate();
  m_iterations++;

  if(m_reset_time_flag) {
    m_model_time_offset = MOOSTime();
    m_time_request(0) = m_model_time_offset;
    m_reset_time_flag = false;
  }

  if(!m_vis_paused) {
    RequestMSEASData();
    VisualizeMSEASData();
    m_appcasting_ready = true;
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: RequestMSEASData

void MSEASOceanModelVisDirect::RequestMSEASData()
{
  m_model_time_request = MOOSTime() + m_model_time_request_offset;

  cout << "filepath requested: " << m_model_filepath << endl;
  cout << "varname requested: " << m_model_varnames_request << endl;
  cout << "utc model time requested: " << m_model_time_request << endl;
  cout << "utc begin time: " << m_model_time_offset << endl;
  cout << "seconds since start (delta): " << (m_model_time_request - m_model_time_offset) << endl;

  m_time_request(1) = m_model_time_request;
  m_time_request(5) = m_grid_num_els;
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

  cout << "time to process " << m_grid_num_els*m_model_varname_cell_request.length() << " requests: " << m_request_time << " seconds." << endl;

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
// Procedure: VisualizeMSEASData

void MSEASOceanModelVisDirect::VisualizeMSEASData()
{
  string spec;
  stringstream ss;
  double cx(0), cy(0);
  double vel_x(0), vel_y(0), vel_z(0);
  int x_idx(-1), y_idx(-1), z_idx(-1), color_idx(-1);
  if (m_map_varname_idx.find("u") != m_map_varname_idx.end()) {
    x_idx = m_map_varname_idx["u"];
  }
  if (m_map_varname_idx.find("v") != m_map_varname_idx.end()) {
    y_idx = m_map_varname_idx["v"];
  }
  if (m_map_varname_idx.find("w") != m_map_varname_idx.end()) {
    z_idx = m_map_varname_idx["w"];
  }
  for(unsigned int i=0; i<m_grid.size(); i++) {
    cx = m_grid.getElement(i).getCenterX();
    cy = m_grid.getElement(i).getCenterY();
    m_vector_field[i].setPosition(cx,cy);
    if (x_idx >= 0) {
      vel_x = m_model_values_return(x_idx,i);
      if (vel_x < 0) {
        vel_x = pow(vel_x*m_vector_scale, m_vector_power);
        if (vel_x > 0) vel_x = -vel_x;
      } else {
        vel_x = pow(vel_x*m_vector_scale, m_vector_power);
      }
    }
    if (y_idx >= 0) {
      vel_y = m_model_values_return(y_idx,i);
      if (vel_y < 0) {
        vel_y = pow(vel_y*m_vector_scale, m_vector_power);
        if (vel_y > 0) vel_y = -vel_y;
      } else {
        vel_y = pow(vel_y*m_vector_scale, m_vector_power);
      }
    }
    if (z_idx >= 0) {
      vel_z = m_model_values_return(z_idx,i);
      if (vel_z < 0) {
        vel_z = pow(vel_z*m_vector_scale, m_vector_power);
        if (vel_z > 0) vel_z = -vel_z;
      } else {
        vel_z = pow(vel_z*m_vector_scale, m_vector_power);
      }
    }
    // set vector in our vector field and post for viewing
    //if (vel_x!=vel_x) vel_x = 0.0;  //check for nan!
    //if (vel_y!=vel_y) vel_y = 0.0;  //actually, doesn't matter for vectors
    //if (vel_z!=vel_z) vel_z = 0.0;
    m_vector_field[i].setVectorXY(vel_x, vel_y);
    ss << m_moosvar_prefix << i;
    m_vector_field[i].set_label(ss.str());
    ss.clear();
    ss.str("");
    spec = m_vector_field[i].get_spec();
    m_Comms.Notify("VIEW_VECTOR", spec);
    // set grid cell in our grid
    if (m_map_varname_idx.find(m_grid_color_var) != m_map_varname_idx.end()) {
      color_idx = m_map_varname_idx[m_grid_color_var];
    } else {
      color_idx = x_idx;
    }
    vel_z = m_model_values_return(color_idx,i);
    if (vel_z!=vel_z) m_grid.setVal(i, 0);  //check for nan values!
    else m_grid.setVal(i, vel_z);
  }
  //post grid for viewing
  ss << m_moosvar_prefix << "GRID";
  m_grid.set_label(ss.str());
  m_grid.set_active(true);
  spec = m_grid.get_spec();
  m_Comms.Notify("VIEW_GRID", spec);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool MSEASOceanModelVisDirect::OnStartUp()
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

  // find the specified global latitude origin for local reference frame -> if unspecified, exit
  if (!m_MissionReader.GetValue("LatOrigin", m_global_lat_origin)) {
    cerr << "MOOS GLOBAL variable latitude origin LatOrigin not specified! Quitting..." << endl;
    return(false);
  }

  // find the specified global longitude origin for local reference frame -> if unspecified, exit
  if (!m_MissionReader.GetValue("LongOrigin", m_global_lon_origin)) {
    cerr << "MOOS GLOBAL variable longitude origin LongOrigin not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid x offset -> if unspecified assume 0
  if (!m_MissionReader.GetConfigurationParam("GRID_X", m_x)) {
    cerr << "Grid X position GRID_X not specified! Assuming 0..." << endl;
    m_x = 0;
  }

  // get the grid y offset -> if unspecified assume 0
  if (!m_MissionReader.GetConfigurationParam("GRID_Y", m_y)) {
    cerr << "Grid Y position GRID_Y not specified! Assuming 0..." << endl;
    m_y = 0;
  }

  // get the grid depth -> if unspecified assume 0
  if (!m_MissionReader.GetConfigurationParam("GRID_DEPTH", m_depth)) {
    cerr << "Depth GRID_DEPTH not specified! Assuming 0..." << endl;
    m_depth = 0;
  }

  // get the grid number of x elements -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_NUM_X", m_grid_num_x_els)) {
    cerr << "Grid number of x elements GRID_NUM_X not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid number of y elements -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_NUM_Y", m_grid_num_y_els)) {
    cerr << "Grid number of y elements GRID_NUM_Y not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid minimum value returned by MSEAS model -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_MIN_VALUE", m_grid_min_value)) {
    cerr << "Grid minimum value GRID_MIN_VALUE not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid maximum value returned by MSEAS model -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_MAX_VALUE", m_grid_max_value)) {
    cerr << "Grid maximum value GRID_MAX_VALUE not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid x spacings -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_DELTA_X", m_grid_delta_x)) {
    cerr << "Grid width spacing GRID_DELTA_X not specified! Quitting..." << endl;
    return(false);
  }

  // get the grid y spacings -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("GRID_DELTA_Y", m_grid_delta_y)) {
    cerr << "Grid height spacing GRID_DELTA_Y not specified! Quitting..." << endl;
    return(false);
  }

  // get path to octave files -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("OCTAVE_PATH", m_octave_path)) {
    cerr << "Path to required octave files OCTAVE_PATH not specified! Quitting..." << endl;
    return(false);
  }

  // get path to MSEAS model file -> if unspecified, exit
  if (!m_MissionReader.GetConfigurationParam("MSEAS_FILEPATH", m_model_filepath)) {
    cerr << "Requested MSEAS filepath MSEAS_FILEPATH not specified! Quitting..." << endl;
    return(false);
  }

  // get list of MSEAS model variable names
  if (!m_MissionReader.GetConfigurationParam("MSEAS_VARNAMES", m_model_varnames_request)) {
    cerr << "Requested MSEAS variable names MSEAS_VARNAMES not specified! Quitting..." << endl;
    return(false);
  }

  // get MSEAS time offset -> if unspecified assume 0
  if (!m_MissionReader.GetConfigurationParam("MSEAS_TIME_OFFSET", m_model_time_request_offset)) {
    cerr << "Requested MSEAS time offset MSEAS_TIME_OFFSET not specified! Quitting..." << endl;
    return(false);
  }

  // get the power value for flow vector visualization -> if unspecified assume 2.0
  if (!m_MissionReader.GetConfigurationParam("VECTOR_POWER", m_vector_power)) {
    cerr << "Vector power VECTOR_POWER not specified! Assuming power 2.0..." << endl;
    m_vector_power = 2.0;
  }

  // get the scaling value for flow vector visualization -> if unspecified assume 1.0
  if (!m_MissionReader.GetConfigurationParam("VECTOR_SCALE", m_vector_scale)) {
    cerr << "Vector scale VECTOR_SCALE not specified! Assuming scale 1.0..." << endl;
    m_vector_scale = 1.0;
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

  // get value by which to color grid -> if unspecified, assume x
  if (!m_MissionReader.GetConfigurationParam("GRID_COLOR_VAR", m_grid_color_var)) {
    cerr << "Grid coloring variable GRID_COLOR_VAR not specified! Assuming 'u'..." << endl;
    m_grid_color_var = "u";
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

  // set up grid positions for MSEAS requests and rendering
  m_geodesy.Initialise(m_global_lat_origin, m_global_lon_origin);
  SetupGrid();

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
// Procedure: SetupGrid

void MSEASOceanModelVisDirect::SetupGrid()
{
  // mseas request setup
  m_grid_num_els = m_grid_num_x_els*m_grid_num_y_els;
  m_model_lon_request.resize(m_grid_num_els);
  m_model_lat_request.resize(m_grid_num_els);
  m_model_depth_request.resize(m_grid_num_els);
  double cur_lon, cur_lat;
  double cur_y = m_y;
  int cur_idx = 0;
  for(int i = 0; i < m_grid_num_y_els; i++) {
    double cur_x = m_x;
    for(int j = 0; j < m_grid_num_x_els; j++) {
      m_geodesy.UTM2LatLong(cur_x, cur_y, cur_lat, cur_lon);
      m_model_lon_request(cur_idx) = cur_lon;
      m_model_lat_request(cur_idx) = cur_lat;
      m_model_depth_request(cur_idx) = m_depth;
      cur_x += m_grid_delta_x;
      cur_idx++;
    }
    cur_y += m_grid_delta_y;
  }

  // grid rendering setup
  double width = m_grid_num_x_els*m_grid_delta_x;
  double height = m_grid_num_y_els*m_grid_delta_y;
  m_vector_field.resize(m_grid_num_els);
  pair<double, double> top_left(m_x, m_y);
  pair<double, double> top_right(m_x+width, m_y);
  pair<double, double> bot_right(m_x+width, m_y+height);
  pair<double, double> bot_left(m_x, m_y+height);
  double cell_size = m_grid_delta_x;
  stringstream ss;
  ss << "pts={" << top_left.first << "," << top_left.second << ": " << top_right.first << "," << top_right.second << ": " << bot_right.first << "," << bot_right.second << ": " << bot_left.first << "," << bot_left.second << "},cell_size=" << cell_size << ",cell_vars=x:0";
  std::string grid_config = ss.str();
  m_grid = string2ConvexGrid(grid_config);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void MSEASOceanModelVisDirect::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // m_Comms.Register("FOOBAR", 0);

  stringstream ss;

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_VARNAME";
  m_varname_moosvar = ss.str();
  m_Comms.Register(m_varname_moosvar, 0);             // name of desired model variable
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_TIME_REQUEST_OFFSET";
  m_time_request_offset_moosvar = ss.str();
  m_Comms.Register(m_time_request_offset_moosvar, 0); // offset in seconds from start of MSEAS model file
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_TIME_OFFSET_RESET";
  m_time_offset_reset_moosvar = ss.str();
  m_Comms.Register(m_time_offset_reset_moosvar, 0);   // resets the offset time of this program
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_GRID_X";
  m_x_moosvar = ss.str();
  m_Comms.Register(m_x_moosvar, 0);   // relocates the x position of the grid
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_GRID_Y";
  m_y_moosvar = ss.str();
  m_Comms.Register(m_y_moosvar, 0);   // relocates the y position of the grid
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_GRID_DEPTH";
  m_depth_moosvar = ss.str();
  m_Comms.Register(m_depth_moosvar, 0);   // relocates the depth of the grid
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_GRID_COLOR_VAR";
  m_color_moosvar = ss.str();
  m_Comms.Register(m_color_moosvar, 0);   // relocates the depth of the grid
  ss.str("");
  ss.clear();

  ss << m_moosvar_prefix << "MSEASOCEANMODELVIS_PAUSED";
  m_paused_moosvar = ss.str();
  m_Comms.Register(m_paused_moosvar, 0);   // relocates the depth of the grid
  ss.str("");
  ss.clear();
}

//---------------------------------------------------------
// Procedure: buildReport

bool MSEASOceanModelVisDirect::buildReport()
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
  return(true);
}
