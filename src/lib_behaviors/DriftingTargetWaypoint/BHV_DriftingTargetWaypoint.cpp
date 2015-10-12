/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTargetWaypoint.cpp                  */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingTargetWaypoint.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_DriftingTargetWaypoint::BHV_DriftingTargetWaypoint(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain)
{
  // Provide a default behavior name
  setParam("name", "BHV_DriftingTargetWaypoint");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingTargetWaypoint::setParam(string param, string val)
{
  if (BHV_DriftingTarget::setParam(param, val))
    return true;

  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());

  if ((param == "point_x")  && (isNumber(val))) {
    m_wpt_x = double_val;
    return(true);
  } else if ((param == "point_y")  && (isNumber(val))) {
    m_wpt_y = double_val;
    return(true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_DriftingTargetWaypoint::onSetParamComplete()
{
  BHV_DriftingTarget::onSetParamComplete();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_DriftingTargetWaypoint::onRunState()
{
  if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;

  BHV_DriftingTarget::addRelativeTargetToFilter(m_wpt_x - m_nav_x, m_wpt_y - m_nav_y);

  return BHV_DriftingTarget::onRunState();
}

