/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ConvexHullTest.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ConvexHullTest.h"
#include <stdlib.h>

using namespace std;

//---------------------------------------------------------
// Constructor

ConvexHullTest::ConvexHullTest() : m_new_pt(false), m_reset(false)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

ConvexHullTest::~ConvexHullTest()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ConvexHullTest::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "CONVEX_HULL_PT") {
      string x = tokStringParse(msg.GetString(), "x", ',', '=');
      string y = tokStringParse(msg.GetString(), "y", ',', '=');
      m_new_x = atof(x.c_str());
      m_new_y = atof(y.c_str());
      m_new_pt = true;
    } else if(msg.GetKey() == "CONVEX_HULL_RESET") {
      m_reset = true;
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

bool ConvexHullTest::OnConnectToServer()
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

bool ConvexHullTest::Iterate()
{
  m_iterations++;

  if (m_new_pt) {
    m_ch.addPoint(m_new_x, m_new_y);
    XYPolygon poly = m_ch.getConvexHullAsPolygon();
    poly.set_label("pnr_tgts");
    poly.set_color("label", "invisible");
    poly.set_color("vertex", "cyan");
    poly.set_color("edge", "cyan");
    poly.set_color("fill", "white");
    poly.set_transparency(0.1);
    poly.set_vertex_size(3);
    Notify("VIEW_POLYGON", poly.get_spec());
    m_new_pt = false;
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ConvexHullTest::OnStartUp()
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

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void ConvexHullTest::RegisterVariables()
{
  // m_Comms.Register("FOOBAR", 0);
  m_Comms.Register("CONVEX_HULL_PT", 0);
  m_Comms.Register("CONVEX_HULL_RESET", 0);
}

