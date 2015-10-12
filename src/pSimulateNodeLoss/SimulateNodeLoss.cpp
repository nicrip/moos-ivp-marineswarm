/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: SimulateNodeLoss.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "SimulateNodeLoss.h"
#include <stdlib.h>
#include <sstream>

using namespace std;

//---------------------------------------------------------
// Constructor

SimulateNodeLoss::SimulateNodeLoss()
{
  m_iterations = 0;
  m_timewarp   = 1;
  m_deploy = false;
  m_start_loss = false;
  m_num_loss = 0;
}

//---------------------------------------------------------
// Destructor

SimulateNodeLoss::~SimulateNodeLoss()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SimulateNodeLoss::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "DEPLOY_ALL") {
      string str_deploy;
      str_deploy = msg.GetString();
      if (str_deploy == "true") m_deploy = true;
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

bool SimulateNodeLoss::OnConnectToServer()
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

bool SimulateNodeLoss::Iterate()
{
  m_iterations++;

  srand (time(NULL));

  if (m_deploy && !m_start_loss) {
    m_start_MOOSTime = MOOSTime();
    m_previous_MOOSTime = MOOSTime();
    m_start_loss = true;
    cout << "START LOSS!" << endl;
  }

  if (m_start_loss) {
    double log_delta = MOOSTime() - m_previous_MOOSTime;
    cout << log_delta << endl;
    cout << m_delta_time << endl;
    cout << m_num_loss << endl;
    cout << m_num_nodes_loss << endl << endl;;

    if ((log_delta > m_delta_time) && (m_num_loss < m_num_nodes_loss)) {
      int node_num = rand() % m_num_nodes + 1;
      while (std::find(m_lost_nodes.begin(), m_lost_nodes.end(), node_num) != m_lost_nodes.end()) {
        node_num = rand() % m_num_nodes + 1;
      }
      m_lost_nodes.push_back(node_num);
      std::ostringstream usm_reset;
      usm_reset << "RELAYED_USM_RESET_NODE_" << node_num;
      std::ostringstream node_name;
      node_name << "NODE_" << node_num;
      Notify("REMOVE_NODE", node_name.str());
      Notify(usm_reset.str(), "x=10000,y=10000");
      m_num_loss++;
      m_previous_MOOSTime = MOOSTime();
    }
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimulateNodeLoss::OnStartUp()
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

  if (!m_MissionReader.GetConfigurationParam("DELTA_TIME", m_delta_time)) {
    cerr << "DELTA_TIME not specified! Quitting..." << endl;
    return(false);
  }

  if (!m_MissionReader.GetConfigurationParam("NUM_NODES_LOSS", m_num_nodes_loss)) {
    cerr << "NUM_NODES_LOSS not specified! Quitting..." << endl;
    return(false);
  }

  if (!m_MissionReader.GetConfigurationParam("NUM_NODES", m_num_nodes)) {
    cerr << "NUM_NODES not specified! Quitting..." << endl;
    return(false);
  }

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void SimulateNodeLoss::RegisterVariables()
{
  // m_Comms.Register("FOOBAR", 0);
  m_Comms.Register("DEPLOY_ALL", 0);
}

