/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: ShoreLogger.cpp                                 */
/*    DATE:                                                 */
/************************************************************/

#include <math.h>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "MBUtils.h"
#include "ShoreLogger.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ShoreLogger::ShoreLogger() : m_first_log(true)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

ShoreLogger::~ShoreLogger()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ShoreLogger::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    //variables from formation metric
    if(msg.GetKey() == "METRIC_TOTAL_DIST")
      m_metric_total_dist = msg.GetDouble();
    if(msg.GetKey() == "METRIC_MEAN_DIST")
      m_metric_mean_dist = msg.GetDouble();
    if(msg.GetKey() == "DEPLOY_ALL") {
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

bool ShoreLogger::OnConnectToServer()
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

bool ShoreLogger::Iterate()
{
  AppCastingMOOSApp::Iterate();

  m_iterations++;

  double log_delta = MOOSTime() - m_previous_MOOSTime;

  if(m_log_shore) {
    if(log_delta >= m_log_time_delta) {
      ofstream log;
      log.open((m_host_community + "_log.csv").c_str(), ios::out | ios::app);
      if(m_first_log) {
        log << setprecision(16) << m_start_MOOSTime << endl;
        m_first_log = false;
      }
      log   << setprecision(16) << (MOOSTime() - m_start_MOOSTime)
            << "," << MOOSTime()
            << setprecision(10) << "," << m_metric_total_dist
            << "," << m_metric_mean_dist
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

bool ShoreLogger::OnStartUp()
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

  if (!m_MissionReader.GetConfigurationParam("LOG_SHORE", m_log_shore)) {
    cerr << "Log boolean LOG_SHORE not specified! Quitting..." << endl;
    return(false);
  }

  if (!m_MissionReader.GetConfigurationParam("LOG_TIME_DELTA", m_log_time_delta)) {
    cerr << "Log time delta LOG_TIME_DELTA not specified! Quitting..." << endl;
    return(false);
  }

  if(m_log_shore) {
    ofstream log;
    log.open((m_host_community + "_log.csv").c_str(), ios::out | ios::app);
    log   << "mission_time_S,"
          << "MOOStime_S,"
          << "metric_total_dist_M,"
          << "metric_mean_dist_M,"
          << "deploy" << endl
          << "start_MOOStime_S" << endl;
    log.close();
  }

  //variables from formation metric
   m_metric_total_dist = 0.;
   m_metric_mean_dist = 0.;
   m_deploy = 0.;

  m_timewarp = GetMOOSTimeWarp();

  RegisterVariables();
  m_previous_MOOSTime = MOOSTime();

  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void ShoreLogger::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  //variables from formation metric
  Register("METRIC_TOTAL_DIST", 0.0);
  Register("METRIC_MEAN_DIST", 0.0);
  Register("DEPLOY_ALL", 0.0);
}

//---------------------------------------------------------
// Procedure: BuildReport

bool ShoreLogger::buildReport()
{
  m_msgs << endl;

  m_msgs << "Configuration:" << endl;
  m_msgs << "\t- shore logging enabled: \t" << m_log_shore << endl;
  m_msgs << "\t- time delta: \t\t\t" << m_log_time_delta << " s" << endl;

  return true;
}
