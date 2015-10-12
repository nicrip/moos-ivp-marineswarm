/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForcesContacts.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingForcesContacts.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_DriftingForcesContacts::BHV_DriftingForcesContacts(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingForces(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_DriftingForcesContacts");

  m_src_x = 0.;
  m_src_y = 0.;
  m_src_clr = 0.;

  addInfoVars("SOURCE_X, SOURCE_Y, SOURCE_CLEAR");
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_DriftingForcesContacts::onRunState()
{
  bool ok1;
  double src_x = getBufferDoubleVal("SOURCE_X", ok1);
  if (!ok1) return NULL;
  double src_y = getBufferDoubleVal("SOURCE_Y", ok1);
  if (!ok1) return NULL;
  double src_clr = getBufferDoubleVal("SOURCE_CLEAR", ok1);
  if (!ok1) return NULL;

  double dist = hypot(src_x - m_src_x, src_y - m_src_y);
  if (dist > 10) {
    BHV_DriftingForces::addRelativeForceSource(src_x - m_nav_x, src_y - m_nav_y, 1.0);
  }

  if (src_clr != m_src_clr) {
    BHV_DriftingForces::clearForceSources();
  }

  m_src_x = src_x;
  m_src_y = src_y;
  m_src_clr = src_clr;

  return BHV_DriftingForces::onRunState();
}

