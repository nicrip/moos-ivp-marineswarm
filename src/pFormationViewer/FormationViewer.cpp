/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: FormationViewer.cpp                             */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "FormationViewer.h"
#include "NodeRecordUtils.h"
#include "XYSegList.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FormationViewer::FormationViewer()
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

FormationViewer::~FormationViewer()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool FormationViewer::OnNewMail(MOOSMSG_LIST &NewMail)
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

bool FormationViewer::HandleNewNodeReport(CMOOSMsg msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();

  // update our auv node position
  m_map_node_records[community_name] = string2NodeRecord(message_value);

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool FormationViewer::OnConnectToServer()
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

bool FormationViewer::Iterate()
{
  AppCastingMOOSApp::Iterate();
  m_iterations++;

  for(vector< pair<string,string> >::iterator it = m_vector_node_pair.begin(); it != m_vector_node_pair.end(); it++) {
    if (m_map_node_records.find((*it).first) == m_map_node_records.end()) {
      // haven't received a node record for our first node
      continue;
    } else if (m_map_node_records.find((*it).second) == m_map_node_records.end()) {
      // haven't received a node record for our second node
      continue;
    } else {
      // received a node record for both nodes
      XYSegList seg_list;
      seg_list.add_vertex(m_map_node_records[(*it).first].getX(),
                          m_map_node_records[(*it).first].getY());

      seg_list.add_vertex(m_map_node_records[(*it).second].getX(),
                          m_map_node_records[(*it).second].getY());

      string label = (*it).first + (*it).second;

      seg_list.set_label(label);
      seg_list.set_active(true);
      seg_list.set_edge_size(2);
      seg_list.set_edge_color("salmon");
      seg_list.set_vertex_color("invisible");
      seg_list.set_label_color("invisible");
      Notify("VIEW_SEGLIST", seg_list.get_spec());
    }
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool FormationViewer::OnStartUp()
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

      if(param == "NODE_PAIR_SEGMENT")
      {
        string sub_values[2] = { "", "" };

        int k = 0;
        for(int i = 0 ; i < value.length() ; i++)
        {
          if(value[i] == ',')
          {
            k = 1;
            continue;
          }

          sub_values[k] += value[i];
        }

        m_vector_node_pair.push_back(make_pair(tolower(sub_values[0]), tolower(sub_values[1])));
      }
    }
  }

  m_timewarp = GetMOOSTimeWarp();

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void FormationViewer::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // m_Comms.Register("FOOBAR", 0);
  Register("NODE_REPORT", 0.0);                       // use node reports to get x/y of each node, instead of passing each individually
}

//---------------------------------------------------------
// Procedure: buildReport

bool FormationViewer::buildReport()
{
  m_msgs << endl << "  Configuration:";
  for(vector< pair<string,string> >::iterator it = m_vector_node_pair.begin(); it != m_vector_node_pair.end(); it++) {
    m_msgs << endl << "    " << (*it).first << " <-> " << (*it).second;
  }

  m_msgs << endl;

  m_msgs << endl << "  State:";
  for(vector< pair<string,string> >::iterator it = m_vector_node_pair.begin(); it != m_vector_node_pair.end(); it++) {
    if (m_map_node_records.find((*it).first) == m_map_node_records.end()) {
      // haven't received a node record for our first node
      m_msgs << endl << "    " << (*it).first << " <-> " << (*it).second << " = FALSE";
      continue;
    } else if (m_map_node_records.find((*it).second) == m_map_node_records.end()) {
      // haven't received a node record for our second node
      m_msgs << endl << "    " << (*it).first << " <-> " << (*it).second << " = FALSE";;
      continue;
    } else {
      // received a node record for both nodes
      m_msgs << endl << "    " << (*it).first << " <-> " << (*it).second << " = TRUE";;
    }
  }

  return(true);
}
