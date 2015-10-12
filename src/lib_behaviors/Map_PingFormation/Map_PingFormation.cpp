/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Map_PingFormation.cpp                           */
/*    DATE: June 2014                                       */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "Map_PingFormation.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

Map_PingFormation::Map_PingFormation(IvPDomain gdomain) : ContactsPing_Formation(gdomain)
{
  setParam("name", "Map_PingFormation");
  addInfoVars("PING_REPORT");
}


//-----------------------------------------------------------
// Procedure: setParam

bool Map_PingFormation::setParam(string param, string param_val) 
{
  if(ContactsPing_Formation::setParam(param, param_val))
    return true;
  
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "add_swarm_element")
  {
    string name;
    double position_x, position_y;
    bool result = MOOSValFromString(name, param_val, "name");
    result = result && MOOSValFromString(position_x, param_val, "x");
    result = result && MOOSValFromString(position_y, param_val, "y");
    m_map_swarm_elements_position[tolower(name)] = XYPoint(position_x, position_y);
    ContactsPing_Formation::addContactToWhiteList(name);
    return result;
  }
  
  return false;
}


//-----------------------------------------------------------
// Procedure: onSetParamComplete

void Map_PingFormation::onSetParamComplete() 
{
  ContactsPing_Formation::onSetParamComplete();
  
  // The contacts map is now seeing from us
  double us_x = m_map_swarm_elements_position[m_us_name].x();
  double us_y = m_map_swarm_elements_position[m_us_name].y();
  
  m_map_contacts.erase(m_us_name);
  m_map_swarm_elements_position.erase(m_us_name);
  
  for(map<string,XYPoint>::iterator contact_position = m_map_swarm_elements_position.begin() ; 
      contact_position != m_map_swarm_elements_position.end() ; 
      contact_position++)
  {
    contact_position->second.set_vertex(contact_position->second.x() - us_x, 
                                        contact_position->second.y() - us_y);
  }
  
  postIntMessage("SWARM_ELEMENTS", m_map_swarm_elements_position.size());
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Map_PingFormation::onRunState() 
{
  if(!ContactsPing_Formation::updatePlatformInfo())
    return 0;
  
  return ContactsPing_Formation::onRunState();
}


//-----------------------------------------------------------
// Procedure: postViewablePoints

void Map_PingFormation::postViewablePoints()
{
  ContactsPing_Formation::postViewablePoints();
}


//-----------------------------------------------------------
// Procedure: postErasablePoints

void Map_PingFormation::postErasablePoints()
{
  ContactsPing_Formation::postErasablePoints();
}


//-----------------------------------------------------------
// Procedure: onRunToIdleState

void Map_PingFormation::onRunToIdleState()
{
  postMessage("PURSUIT", 0);
  postErasablePoints();
}
