/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTgtWptContacts.cpp                  */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingTgtWptContacts.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_DriftingTgtWptContacts::BHV_DriftingTgtWptContacts(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  setParam("name", "BHV_DriftingTgtWptContacts");

  m_curr_wpt_x = 0;
  m_curr_wpt_y = 0;
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingTgtWptContacts::setParam(string param, string val)
{
  // multiple inheritance is ugly, but other options are worse!
  if (BHV_DriftingTarget::setParam(param, val))
    return true;
  if (BHV_AcousticPingPlanner::setParam(param, val))
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
  } else if (param == "contact_whitelist") {
    m_contact_whitelist_namevector = parseString(val, ',');
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

void BHV_DriftingTgtWptContacts::onSetParamComplete()
{
  BHV_DriftingTarget::onSetParamComplete();
  BHV_AcousticPingPlanner::onSetParamComplete();

  // add whitelist contacts to our ping manager
  for (unsigned int i=0; i<m_contact_whitelist_namevector.size(); i++) {
    BHV_AcousticPingPlanner::addContactToWhiteList(m_contact_whitelist_namevector[i]);
  }
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_DriftingTgtWptContacts::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_DriftingTgtWptContacts::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_DriftingTgtWptContacts::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_DriftingTgtWptContacts::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_DriftingTgtWptContacts::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_DriftingTgtWptContacts::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_DriftingTgtWptContacts::onRunState()
{
  if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  if ((m_wpt_x != m_curr_wpt_x) || (m_wpt_y != m_curr_wpt_y)) {
    postWMessage("WAYPOINT CHANGED!!!");
    BHV_DriftingTarget::addRelativeTargetToFilter(m_wpt_x - BHV_DriftingTarget::m_nav_x, m_wpt_y - BHV_DriftingTarget::m_nav_y);
    m_curr_wpt_x = m_wpt_x;
    m_curr_wpt_y = m_wpt_y;
  }

  return BHV_DriftingTarget::onRunState();
}

