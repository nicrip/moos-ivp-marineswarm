/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Trail_PingFormation.cpp                         */
/*    DATE: June 2014                                       */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "Trail_PingFormation.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

Trail_PingFormation::Trail_PingFormation(IvPDomain gdomain) : OneContact_PingFormation(gdomain)
{
  setParam("name", "Trail_PingFormation");
  m_trail_dx = 0.;
  m_trail_dy = 0.;
}


//-----------------------------------------------------------
// Procedure: setParam

bool Trail_PingFormation::setParam(string param, string param_val) 
{
  if(OneContact_PingFormation::setParam(param, param_val))
    return true;
    
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));
  
  if(param == "trail_dx" && isNumber(param_val))
  {
    m_trail_dx = dval;
    return true;
  }
  
  if(param == "trail_dy" && isNumber(param_val))
  {
    m_trail_dy = dval;
    return true;
  }
  
  return false;
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Trail_PingFormation::onRunState() 
{
  if(!OneContact_PingFormation::updatePlatformInfo())
    return 0;
  
  double assumed_contact_relative_x = getContact().GetRelativeX();
  double assumed_contact_relative_y = getContact().GetRelativeY();
  
  // Calculating the target point
  double targetpoint_position_x, targetpoint_position_y;
  projectPoint(relAng(0, 0, m_trail_dx, m_trail_dy), hypot(m_trail_dx, m_trail_dy), 
                assumed_contact_relative_x, assumed_contact_relative_y, 
                targetpoint_position_x, targetpoint_position_y);
  m_relative_target_point.set_vertex(targetpoint_position_x, targetpoint_position_y);
  filterCentroidTargetpoint();
  return OneContact_PingFormation::onRunState();
}
