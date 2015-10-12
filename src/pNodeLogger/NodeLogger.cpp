/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: NodeLogger.cpp                                  */
/*    DATE:                                                 */
/************************************************************/

#include <math.h>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "MBUtils.h"
#include "NodeRecordUtils.h"
#include "NodeLogger.h"

using namespace std;

//---------------------------------------------------------
// Constructor

NodeLogger::NodeLogger() : m_first_log(true)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

NodeLogger::~NodeLogger()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool NodeLogger::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    //variables from controller
    if(msg.GetKey() == "NAV_X")
      m_nav_x = msg.GetDouble();
    if(msg.GetKey() == "NAV_Y")
      m_nav_y = msg.GetDouble();
    if(msg.GetKey() == "NAV_DEPTH")
      m_nav_depth = msg.GetDouble();
    if(msg.GetKey() == "NAV_HEADING")
      m_nav_heading = msg.GetDouble();
    if(msg.GetKey() == "NAV_SPEED")
      m_nav_speed = msg.GetDouble();
    //variables from current simulator
    if(msg.GetKey() == "DRIFT_X")
      m_drift_x = msg.GetDouble();
    if(msg.GetKey() == "DRIFT_Y")
      m_drift_y = msg.GetDouble();
    //variables from consumption monitor
    if(msg.GetKey() == "POWER_NOMINAL")
      m_power_nominal = msg.GetDouble();
    if(msg.GetKey() == "POWER_PROP")
      m_power_prop = msg.GetDouble();
    if(msg.GetKey() == "CONSO_NOMINAL")
      m_conso_nominal = msg.GetDouble();
    if(msg.GetKey() == "CONSO_PROP")
      m_conso_prop = msg.GetDouble();
    if(msg.GetKey() == "CONSO_COMM")
      m_conso_comm = msg.GetDouble();
    if(msg.GetKey() == "CONSO_ALL")
      m_conso_all = msg.GetDouble();
    if(msg.GetKey() == "ESTIMATION_DURATION_DAYS")
      m_estimation_duration_days = msg.GetDouble();
    //variables from drifting target behavior BHV_DriftingTarget
    if(msg.GetKey() == "DRIFTING_TARGET_TIME_DRIFTING")
      m_time_drifting = msg.GetDouble();
    if(msg.GetKey() == "DRIFTING_TARGET_TIME_THRUSTING")
      m_time_thrusting = msg.GetDouble();
    if(msg.GetKey() == "DRIFTING_TARGET_DRIFTING_RATIO")
      m_drifting_ratio = msg.GetDouble();
    if(msg.GetKey() == "DEPLOY") {
      string str_deploy;
      str_deploy = msg.GetString();
      if (str_deploy == "true") m_deploy = 1;
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

bool NodeLogger::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   m_start_MOOSTime = MOOSTime();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool NodeLogger::Iterate()
{
  AppCastingMOOSApp::Iterate();

  m_iterations++;

  double log_delta = MOOSTime() - m_previous_MOOSTime;

  if(m_log_node) {
    if(m_first_log) {
      m_previous_nav_x = m_nav_x;
      m_previous_nav_y = m_nav_y;
    }
    m_total_distance += hypot(m_previous_nav_x - m_nav_x, m_previous_nav_y - m_nav_y);
    if (m_nav_speed > 0) {
      m_thrusting_distance += hypot(m_previous_nav_x - m_nav_x, m_previous_nav_y - m_nav_y);
    } else {
      m_drifting_distance += hypot(m_previous_nav_x - m_nav_x, m_previous_nav_y - m_nav_y);
    }
    m_dist_drifting_ratio = m_drifting_distance/m_total_distance;
    m_previous_nav_x = m_nav_x;
    m_previous_nav_y = m_nav_y;

    if(log_delta >= m_log_time_delta) {
      ofstream log;
      log.open((m_log_folder + m_host_community + "_log.csv").c_str(), ios::out | ios::app);
      if(m_first_log) {
        log << setprecision(16) << m_start_MOOSTime << endl;
        m_first_log = false;
      }
      log   << setprecision(16) << (MOOSTime() - m_start_MOOSTime)
            << "," << MOOSTime()
            << setprecision(10) << "," << m_nav_x
            << "," << m_nav_y
            << "," << m_nav_depth
            << "," << m_nav_heading
            << "," << m_nav_speed
            << "," << m_drift_x
            << "," << m_drift_y
            << "," << (m_conso_nominal / 3600.)
            << "," << (m_conso_prop / 3600.)
            << "," << (m_conso_comm / 3600.)
            << "," << (m_conso_all / 3600.)
            << "," << m_estimation_duration_days
            << "," << m_time_drifting
            << "," << m_time_thrusting
            << "," << m_drifting_ratio
            << "," << m_drifting_distance
            << "," << m_thrusting_distance
            << "," << m_total_distance
            << "," << m_dist_drifting_ratio
            << "," << m_deploy << endl;
      log.close();
      m_previous_MOOSTime = MOOSTime();
    }
  }

  AppCastingMOOSApp::PostReport();

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool NodeLogger::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
  {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++)
    {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
    }
  }

  if (!m_MissionReader.GetConfigurationParam("LOG_TIME_DELTA", m_log_time_delta)) {
    cerr << "Log time delta LOG_TIME_DELTA not specified! Quitting..." << endl;
    return(false);
  }

  if (!m_MissionReader.GetConfigurationParam("LOG_NODE", m_log_node)) {
    cerr << "Log boolean LOG_NODE not specified! Quitting..." << endl;
    return(false);
  }

  if (!m_MissionReader.GetConfigurationParam("LOG_FOLDER", m_log_folder)) {
    cerr << "Log folder LOG_FOLDER not specified! Quitting..." << endl;
    return(false);
  }

  if(m_log_node) {
    ofstream log;
    log.open((m_log_folder + m_host_community + "_log.csv").c_str(), ios::out | ios::app);
    log   << "mission_time_S,"
          << "MOOStime_S,"
          << "x_M,"
          << "y_M,"
          << "depth_M,"
          << "heading_DEG,"
          << "speed_M/S,"
          << "drift_x_M/S,"
          << "drift_y_M/S,"
          << "consum_nominal_WH,"
          << "consum_propulsion_WH,"
          << "consum_communication_WH,"
          << "consum_total_WH,"
          << "duration_estimation_DAYS,"
          << "time_drifting_S,"
          << "time_thrusting_S,"
          << "time_drifting_ratio_%,"
          << "distance_drifting_M,"
          << "distance_thrusting_M,"
          << "distance_total_M,"
          << "distance_drifting_ratio_%,"
          << "deploy" << endl
          << "start_MOOStime_S" << endl;
    log.close();
  }

  //variables from controller
  m_nav_x = 0.;
  m_nav_y = 0.;
  m_nav_depth = 0.;
  m_nav_heading = 0.;
  m_nav_speed = 0.;
  m_previous_nav_x = 0.;
  m_previous_nav_y = 0.;
  //variables from current simulator
  m_drift_x = 0.;
  m_drift_y = 0.;
  //variables from consumption monitor
  m_conso_nominal = 0.;
  m_conso_prop = 0.;
  m_conso_comm = 0.;
  m_conso_all = 0.;
  m_power_nominal = 0.;
  m_power_prop = 0.;
  m_estimation_duration_days = 0.;
  //variables from drifting target behavior BHV_DriftingTarget
  m_time_drifting = 0.;
  m_time_thrusting = 0.;
  m_drifting_ratio = 0.;
  //distance_metrics
  m_drifting_distance = 0.;
  m_thrusting_distance = 0.;
  m_total_distance = 0.;
  m_dist_drifting_ratio = 0.;
  m_deploy = 0.;

  m_timewarp = GetMOOSTimeWarp();

  RegisterVariables();
  m_previous_MOOSTime = MOOSTime();

  return true;
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void NodeLogger::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  //variables from controller
  Register("NAV_X", 0.0);
  Register("NAV_Y", 0.0);
  Register("NAV_DEPTH", 0.0);
  Register("NAV_SPEED", 0.0);
  Register("NAV_HEADING", 0.0);
  //variables from current simulator
  Register("DRIFT_X", 0.0);
  Register("DRIFT_Y", 0.0);
  //variables from consumption monitor
  Register("POWER_NOMINAL", 0.0);
  Register("POWER_PROP", 0.0);
  Register("CONSO_NOMINAL", 0.0);
  Register("CONSO_PROP", 0.0);
  Register("CONSO_COMM", 0.0);
  Register("CONSO_ALL", 0.0);
  Register("ESTIMATION_DURATION_DAYS", 0.0);
  //variables from drifting target behavior BHV_DriftingTarget
  Register("DRIFTING_TARGET_TIME_DRIFTING", 0.0);
  Register("DRIFTING_TARGET_TIME_THRUSTING", 0.0);
  Register("DRIFTING_TARGET_DRIFTING_RATIO", 0.0);
  Register("DEPLOY", 0.0);
}

//---------------------------------------------------------
// Procedure: BuildReport

bool NodeLogger::buildReport()
{
  m_msgs << endl;

  m_msgs << "Configuration:" << endl;
  m_msgs << "\t- node logging enabled: \t" << m_log_node << endl;
  m_msgs << "\t- log folder: \t\t\t" << m_log_folder << endl;
  m_msgs << "\t- time delta: \t\t\t" << m_log_time_delta << " s" << endl;

  return true;
}
