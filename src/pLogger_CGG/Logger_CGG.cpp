/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: Logger_CGG.cpp                                   */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#include <math.h>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "MBUtils.h"
#include "Logger_CGG.h"
#include "NodeRecordUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Logger::Logger()
{
  m_nav_x = 0.;
  m_nav_y = 0.;
  m_previous_nav_x = 0.;
  m_previous_nav_y = 0.;
  m_drift_distance = 0.;
  m_nav_depth = 0.;
  m_active_speed = 0.;
  m_drift_x = 0.;
  m_drift_y = 0.;
  m_conso_nominal = 0.;
  m_conso_prop = 0.;
  m_conso_comm = 0.;
  m_conso_all = 0.;
  m_ratio_recording = 0.;
  m_estimation_duration_days = 0.;
  m_starting_time = 0.;
  m_previous_log_date = 0.;
  m_distance_over_ground = 0.;
  m_distance_over_ground_computable = false;
  m_active_distance = 0.;
  
  m_enable_logging = false;
  m_log_time_gap = 15.;
}

//---------------------------------------------------------
// Destructor

Logger::~Logger()
{
  
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Logger::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    bool handled = false;
    
    if(msg.GetKey() == "NAV_X")
      m_nav_x = msg.GetDouble();
    
    if(msg.GetKey() == "NAV_Y")
      m_nav_y = msg.GetDouble();
    
    if(msg.GetKey() == "NAV_DEPTH")
      m_nav_depth = msg.GetDouble();
    
    if(msg.GetKey() == "NAV_SPEED")
      m_active_speed = msg.GetDouble();
    
    if(msg.GetKey() == "DRIFT_X")
      m_drift_x = msg.GetDouble();
    
    if(msg.GetKey() == "DRIFT_Y")
      m_drift_y = msg.GetDouble();
    
    if(msg.GetKey() == "CONSO_NOMINAL")
      m_conso_nominal = msg.GetDouble();
    
    if(msg.GetKey() == "CONSO_PROP")
      m_conso_prop = msg.GetDouble();
    
    if(msg.GetKey() == "CONSO_COMM")
      m_conso_comm = msg.GetDouble();
    
    if(msg.GetKey() == "CONSO_ALL")
      m_conso_all = msg.GetDouble();
    
    if(msg.GetKey() == "RATIO_RECORDING")
      m_ratio_recording = msg.GetDouble();
    
    if(msg.GetKey() == "ESTIMATION_DURATION_DAYS")
      m_estimation_duration_days = msg.GetDouble();
    
    if(!handled)
      cout << "Unhandled Mail: " << msg.GetKey() << endl;
    
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

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Logger::OnConnectToServer()
{
  RegisterVariables();
  m_starting_time = MOOSTime();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Logger::Iterate()
{
  AppCastingMOOSApp::Iterate();
  double delay_log = MOOSTime() - m_previous_log_date;
  
  if(m_enable_logging)
  {
    double drift_mag = hypot(m_drift_x, m_drift_y);
    m_drift_distance += drift_mag * delay_log;
    m_active_distance += m_active_speed * delay_log;
    
    if(m_distance_over_ground_computable)
      m_distance_over_ground += hypot(m_previous_nav_x - m_nav_x, m_previous_nav_y - m_nav_y);
      
    m_previous_nav_x = m_nav_x;
    m_previous_nav_y = m_nav_y;
    m_distance_over_ground_computable = true;
    
    if(delay_log > 5)
    {
      ofstream logs;
      logs.open(("logs/" + m_host_community + "_logs.csv").c_str(), ios::out | ios::app);
      logs << setprecision(16) 
            << (MOOSTime() - m_starting_time)
            << "," << MOOSTime() 
            << setprecision(10)
            << "," << m_nav_x
            << "," << m_nav_y
            << "," << m_nav_depth
            << "," << m_active_speed
            << "," << m_active_distance
            << "," << drift_mag
            << "," << m_drift_distance
            << "," << m_distance_over_ground
            << "," << (m_conso_nominal / 3600.)
            << "," << (m_conso_prop / 3600.)
            << "," << (m_conso_comm / 3600.)
            << "," << (m_conso_all / 3600.)
            << "," << m_ratio_recording
            << "," << m_estimation_duration_days << endl;
      logs.close();
      m_previous_log_date = MOOSTime();
    }
  }

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Logger::OnStartUp()
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

      if(param == "LOG_TIME_GAP")
        m_log_time_gap = atof(value.c_str());

      else if(param == "ENABLE_LOGGING")
        setBooleanOnString(m_enable_logging, value);
    }
  }
  
  if(m_enable_logging)
  {
    ofstream logs;
    logs.open(("logs/" + m_host_community + "_logs.csv").c_str(), ios::out | ios::app);
    logs << "mission_time_S,"
         << "MOOStime_S,"
         << "x_M,"
         << "y_M,"
         << "depth_M,"
         << "speed_M/S,"
         << "active_distance_M,"
         << "drift_M/S,"
         << "drift_distance_M,"
         << "total_distance_M,"
         << "cons_nominal_WH,"
         << "cons_propulsion_WH,"
         << "cons_communication_WH,"
         << "cons_total_WH,"
         << "recording_ratio_%,"
         << "duration_estimation_DAYS" << endl;
    logs.close();
  }
  
  RegisterVariables();
  m_previous_log_date = MOOSTime();
  
  return true;
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Logger::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("NAV_X", 0.0);
  Register("NAV_Y", 0.0);
  Register("NAV_DEPTH", 0.0);
  Register("NAV_SPEED", 0.0);
  Register("DRIFT_X", 0.0);
  Register("DRIFT_Y", 0.0);
  Register("CONSO_NOMINAL", 0.0);
  Register("CONSO_PROP", 0.0);
  Register("CONSO_COMM", 0.0);
  Register("CONSO_ALL", 0.0);
  Register("RATIO_RECORDING", 0.0);
  Register("ESTIMATION_DURATION_DAYS", 0.0);
}

//---------------------------------------------------------
// Procedure: BuildReport

bool Logger::buildReport()
{
  m_msgs << endl;
  
  m_msgs << "Configuration:" << endl;
  m_msgs << "\t- log time gap: \t\t" << m_log_time_gap << " W" << endl;
  m_msgs << "\t- enable: \t\t" << m_enable_logging << " W" << endl;
  
  return true;
}
