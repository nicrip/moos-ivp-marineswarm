/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Inspired by Map_PingFormation by Simon Rohou    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AcousticPingPlanner.cpp                     */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_AcousticPingPlanner.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_AcousticPingPlanner::BHV_AcousticPingPlanner(IvPDomain domain) :
  IvPBehavior(domain), BHV_ManageAcousticPing(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_AcousticPingPlanner");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_AcousticPingPlanner::setParam(string param, string val)
{
  if (BHV_ManageAcousticPing::setParam(param, val))
    return true;

  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());

  if (param == "node_offsets") {
    string name;
    double offset_x, offset_y;
    bool result = MOOSValFromString(name, val, "name");
    result = result && MOOSValFromString(offset_x, val, "x");
    result = result && MOOSValFromString(offset_y, val, "y");
    m_swarm_plan[tolower(name)] = XYPoint(offset_x, offset_y);
    BHV_ManageAcousticPing::addContactToWhiteList(name);
    return result;
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_AcousticPingPlanner::onSetParamComplete()
{
  BHV_ManageAcousticPing::onSetParamComplete();

  m_original_swarm_plan = m_swarm_plan;

  // go through the plan of node offsets, changing all nodes to positions relative to ourselves (the node this behavior is running on) - also, remove ourselves from our plan list and acoustic ping manager whitelist
  double us_x = m_swarm_plan[tolower(m_us_name)].x();
  double us_y = m_swarm_plan[tolower(m_us_name)].y();

  BHV_ManageAcousticPing::removeContactFromWhiteList(tolower(m_us_name));
  m_swarm_plan.erase(tolower(m_us_name));

  for(map<string,XYPoint>::iterator node_offset = m_swarm_plan.begin(); node_offset != m_swarm_plan.end(); ++node_offset) {
    node_offset->second.set_vertex(node_offset->second.x() - us_x, node_offset->second.y() - us_y); // other nodes now relative to our planned position
  }

  postIntMessage("ACOUSTIC_PING_PLANNER_NUM_NODES", m_swarm_plan.size());
}
