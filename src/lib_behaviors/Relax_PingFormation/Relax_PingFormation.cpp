/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Relax_PingFormation.cpp                         */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "Relax_PingFormation.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

Relax_PingFormation::Relax_PingFormation(IvPDomain gdomain) : OneContact_PingFormation(gdomain)
{
  setParam("name", "Relax_PingFormation");
  m_range = 300.;
}


//-----------------------------------------------------------
// Procedure: setParam

bool Relax_PingFormation::setParam(string param, string param_val) 
{
  if(OneContact_PingFormation::setParam(param, param_val))
    return true;
    
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "range" && isNumber(param_val))
    return (m_range = dval);
  
  return false;
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Relax_PingFormation::onRunState() 
{
  if(!OneContact_PingFormation::updatePlatformInfo())
    return 0;
  
  double assumed_contact_relative_x = getContact().GetRelativeX();
  double assumed_contact_relative_y = getContact().GetRelativeY();
  double range = hypot(assumed_contact_relative_x, assumed_contact_relative_y);
  double angle = relAng(assumed_contact_relative_x, assumed_contact_relative_y, 0, 0);
  
  // Calculating the target point
  double targetpoint_position_x, targetpoint_position_y;
  projectPoint(angle360(angle), m_range, 
                assumed_contact_relative_x, assumed_contact_relative_y,
                targetpoint_position_x, targetpoint_position_y);
  m_relative_target_point.set_vertex(targetpoint_position_x, targetpoint_position_y);
  
  return OneContact_PingFormation::onRunState();
}
