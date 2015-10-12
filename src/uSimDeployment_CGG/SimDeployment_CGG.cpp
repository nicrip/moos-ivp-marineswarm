/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimDeployment_CGG.cpp                            */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#include <iterator>
#include <fstream>
#include "MBUtils.h"
#include "SimDeployment_CGG.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SimDeployment::SimDeployment()
{
  m_nav_x = 0;
  m_nav_y = 0;
  m_current_launch_point = -1;
}


//---------------------------------------------------------
// Destructor

SimDeployment::~SimDeployment()
{
  
}


//---------------------------------------------------------
// Procedure: OnNewMail

bool SimDeployment::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    
    if(msg.GetKey() == "NAV_X")
      m_nav_x = msg.GetDouble();
    
    if(msg.GetKey() == "NAV_Y")
      m_nav_y = msg.GetDouble();
    
    if(msg.GetKey() == "WPT_INDEX")
    {
      m_current_launch_point = (int)msg.GetDouble();
      for(vector<string>::iterator deploy_vehicle = m_map_launches[m_current_launch_point].begin() ; 
        deploy_vehicle != m_map_launches[m_current_launch_point].end() ; 
          deploy_vehicle++)
      {
        ostringstream strs;
        strs << "x=" << m_nav_x <<
                ",y=" << m_nav_y <<
                ",z=0";
        Notify("USM_RESET_" + toupper(*deploy_vehicle), strs.str());
        Notify("DEPLOY_" + toupper(*deploy_vehicle), "true");
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

  return true;
}


//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SimDeployment::OnConnectToServer()
{
  RegisterVariables();
  return true;
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SimDeployment::Iterate()
{
  AppCastingMOOSApp::Iterate();  
  AppCastingMOOSApp::PostReport();
  return true;
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimDeployment::OnStartUp()
{
  int current_launch_point = -1;
  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
  {
    sParams.reverse();
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++)
    {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
      
      if(param == "LAUNCH_POINT")
        current_launch_point = atoi(value.c_str());

      if(param == "DEPLOY_VEHICLE")
      {
        if(current_launch_point == -1)
          return false;
        
        m_map_launches[current_launch_point].push_back(value);
      }
    }
  }
  
  RegisterVariables();
  
  return true;
}


//---------------------------------------------------------
// Procedure: RegisterVariables

void SimDeployment::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("NAV_X", 0.0);
  Register("NAV_Y", 0.0);
  Register("WPT_INDEX", 0.0);
  
  for(map<int, vector<string> >::iterator launch_point = m_map_launches.begin() ; 
      launch_point != m_map_launches.end() ; 
      launch_point++)
  {
    for(vector<string>::iterator deploy_vehicle = launch_point->second.begin() ; 
        deploy_vehicle != launch_point->second.end() ; 
        deploy_vehicle++)
    {
      Register(*deploy_vehicle, 0.0);
    }
  }
}


//---------------------------------------------------------
// Procedure: buildReport

bool SimDeployment::buildReport()
{
  m_msgs << endl;
  
  m_msgs << "  Current launch point: " << m_current_launch_point << endl;
  
  if(m_map_launches.size() != 0)
  {
    m_msgs << "  Mission deployment plan:" << endl;

    for(map<int, vector<string> >::iterator launch_point = m_map_launches.begin() ; 
      launch_point != m_map_launches.end() ; 
        launch_point++)
    {
      m_msgs << endl << "\tLaunch Point " << launch_point->first << ": " << endl;
      
      for(vector<string>::iterator deploy_vehicle = launch_point->second.begin() ; 
        deploy_vehicle != launch_point->second.end() ; 
          deploy_vehicle++)
      {
        m_msgs << "\t  - " << *deploy_vehicle << endl;
      }
    }
  }
  
  else
    m_msgs << "  Shooters deployed." << endl;

  return true;
}
