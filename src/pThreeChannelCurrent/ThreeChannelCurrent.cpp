/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: ThreeChannelCurrent.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ThreeChannelCurrent.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ThreeChannelCurrent::ThreeChannelCurrent()
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

ThreeChannelCurrent::~ThreeChannelCurrent()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ThreeChannelCurrent::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "NAV_X")
      m_nav_x = msg.GetDouble();
    if(msg.GetKey() == "NAV_Y")
      m_nav_y = msg.GetDouble();

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

bool ThreeChannelCurrent::OnConnectToServer()
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

bool ThreeChannelCurrent::Iterate()
{
  m_iterations++;

  if (m_nav_y > 150.0) {
    Notify("DRIFT_X", 0.1);
    Notify("DRIFT_Y", 0.0);
  } else if (m_nav_y < -150.0) {
    Notify("DRIFT_X", -0.1);
    Notify("DRIFT_Y", 0.0);
  } else {
    Notify("DRIFT_X", 0.0);
    Notify("DRIFT_Y", 0.0);
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ThreeChannelCurrent::OnStartUp()
{
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

  m_nav_x = 0.;
  m_nav_y = 0.;

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void ThreeChannelCurrent::RegisterVariables()
{
  // m_Comms.Register("FOOBAR", 0);
  Register("NAV_X", 0.0);
  Register("NAV_Y", 0.0);
}

