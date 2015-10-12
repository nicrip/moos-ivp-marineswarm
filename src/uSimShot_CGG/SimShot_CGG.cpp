/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimShot_CGG.cpp                                  */
/*   DATE: june 2014                                        */
/* ******************************************************** */

#include <iterator>
#include <fstream>
#include "MBUtils.h"
#include "SimShot_CGG.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SimShot::SimShot()
{
  m_nav_x = 0;
  m_nav_y = 0;
  m_deploy_shooters = false;
  m_order_next_shot = false;
  m_time_between_shots = 5.0;
  m_next_shooter = "?";
  m_shot_number = 0;
}


//---------------------------------------------------------
// Destructor

SimShot::~SimShot()
{
  
}


//---------------------------------------------------------
// Procedure: OnNewMail

bool SimShot::OnNewMail(MOOSMSG_LIST &NewMail)
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
    
    if(msg.GetKey() == "DEPLOY")
      setBooleanOnString(m_deploy_shooters, msg.GetString());
    
    if(msg.GetKey() == "SHOT_ORDER" && m_deploy_shooters)
    {
      bool shot_order = false;
      setBooleanOnString(shot_order, msg.GetString());
      
      if(shot_order)
      {
        m_order_next_shot = true;
        m_shot_date = MOOSTime();
        DisplayShot();
        m_shot_number ++;
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

bool SimShot::OnConnectToServer()
{
  RegisterVariables();
  return true;
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SimShot::Iterate()
{
  AppCastingMOOSApp::Iterate();  
  AppCastingMOOSApp::PostReport();
  
  if(m_order_next_shot && MOOSTime() - m_shot_date > m_time_between_shots)
  {
    m_order_next_shot = false;
    Notify("SHOT_ORDER_" + m_next_shooter, "true");
  }
    
  return true;
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimShot::OnStartUp()
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
      
      if(param == "TIME_BETWEEN_SHOTS")
        m_time_between_shots = atof(value.c_str());
      
      if(param == "NEXT_SHOOTER")
        m_next_shooter = value;
    }
  }
  
  RegisterVariables(); 
  return true;
}


//---------------------------------------------------------
// Procedure: RegisterVariables

void SimShot::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("NAV_X", 0.0);
  Register("NAV_Y", 0.0);
  Register("DEPLOY", 0.0);
  Register("SHOT_ORDER", 0.0);
}


//---------------------------------------------------------
// Procedure: buildReport

bool SimShot::buildReport()
{
  m_msgs << endl;
  m_msgs << "  uSimShot is running" << endl;
  
  if(m_deploy_shooters)
    m_msgs << "  Shooter is deployed" << endl;
    
  m_msgs << "  Shot number: " << m_shot_number << endl;
  m_msgs << "  Next shooter: " << m_next_shooter << endl;
  
  return true;
}


//---------------------------------------------------------
// Procedure: DisplayShot

void SimShot::DisplayShot()
{
  // Display circles around emitters:
  std::ostringstream s;
  s << "x=" << m_nav_x
    << ",y=" << m_nav_y
    << ",radius=" << 3000.0 
    << ",duration=" << 2.0 
    << ",fill=" << 0.9
    << ",label=" << "shot_"
    << ",edge_color=" << "white"
    << ",fill_color=" << "white" 
    << ",time=" << setprecision(16) << MOOSTime()
    << ",edge_size=" << 1.0;
  Notify("VIEW_RANGE_PULSE", s.str());
}
