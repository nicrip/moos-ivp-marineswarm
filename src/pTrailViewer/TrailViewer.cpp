/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: TrailViewer.cpp                                 */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "TrailViewer.h"
#include <math.h>

using namespace std;

//---------------------------------------------------------
// Constructor

TrailViewer::TrailViewer() : m_distance(0.0), m_init_x(false), m_init_y(false), m_init_name(false)
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

TrailViewer::~TrailViewer()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool TrailViewer::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "NAV_X") {
      double x = msg.GetDouble();
      double y = m_pos.y();
      m_pos.set_vertex(x, y);
      m_name = msg.GetCommunity();
      m_init_x = true;
    } else if(msg.GetKey() == "NAV_Y") {
      double y = msg.GetDouble();
      double x = m_pos.x();
      m_pos.set_vertex(x, y);
      m_name = msg.GetCommunity();
      m_init_y = true;
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

bool TrailViewer::OnConnectToServer()
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

bool TrailViewer::Iterate()
{
  m_iterations++;

  if (m_init_x && m_init_y) {
    if(!m_init_name) {
      // set random color for targets and circles - average with white for a pleasant 'light pastel' color
      int sum = 0;
      for (unsigned int i = 0; i < m_name.size(); i++) {
        sum += m_name[i];
      }
      srand(sum + time(NULL));
      double r, g, b;
      r = (((double)rand()/(double)RAND_MAX)+1)/2.0;
      g = (((double)rand()/(double)RAND_MAX)+1)/2.0;
      b = (((double)rand()/(double)RAND_MAX)+1)/2.0;
      ColorPack color(r,g,b);
      m_color = color;

      m_trail.set_color("edge", m_color);
      m_trail.set_vertex_color("invisible");
      m_trail.set_label_color("invisible");
      m_trail.set_edge_size(3);
      m_trail.set_label(m_name + "_trail");

      m_init_name = true;
    }

    if (m_trail.size() == 0) {
      m_trail.add_vertex(m_pos);
      m_prev_pos = m_pos;
    } else {
      if (hypot(m_pos.x()-m_prev_pos.x(), m_pos.y()-m_prev_pos.y()) >= m_distance) {
        m_trail.add_vertex(m_pos);
        m_prev_pos = m_pos;
        Notify("VIEW_SEGLIST", m_trail.get_spec());
      }
    }
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool TrailViewer::OnStartUp()
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

  if (!m_MissionReader.GetConfigurationParam("DISTANCE", m_distance)) {
    cerr << "DISTANCE not specified! Assuming 10..." << endl;
    m_distance = 10.0;
  }

  m_pos.set_vertex(0.0, 0.0);

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void TrailViewer::RegisterVariables()
{
  m_Comms.Register("NAV_X", 0);
  m_Comms.Register("NAV_Y", 0);
}

