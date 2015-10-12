/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForces.cpp                          */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingForces.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_DriftingForces::BHV_DriftingForces(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_DriftingForces");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Initialize config variables
  m_speed = 1.;
  m_max_force = 100.;
  m_capture_force = 3.;
  m_slip_force = 15.;
  m_drifting_force = 30.;
  m_display_force_sources = true;
  m_display_total_force = true;
  m_display_partial_forces = true;
  m_display_statistics = true;
  m_display_drift = true;

  // Initialize state variables
  m_nav_heading = 0.;
  m_nav_depth = 0.;
  m_nav_x = 0.;
  m_nav_y = 0.;
  m_energy = 0.;
  m_drift_x = 0.;
  m_drift_y = 0.;
  m_drifting = true;
  m_rel_summed_force_x = 0.;
  m_rel_summed_force_y = 0.;

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_DEPTH, NAV_HEADING, CONSO_ALL, DRIFT_X, DRIFT_Y");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingForces::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  bool non_neg_number = (isNumber(val) && (double_val >= 0));

  if ((param == "capture_force") && non_neg_number) {
    m_capture_force = double_val;
    return(true);
  } else if ((param == "slip_force") && non_neg_number) {
    m_slip_force = double_val;
    return(true);
  } else if ((param == "drifting_force") && non_neg_number) {
    m_drifting_force = double_val;
    return(true);
  } else if ((param == "speed") && non_neg_number) {
    m_speed = double_val;
    return(true);
  } else if ((param == "max_force") && non_neg_number) {
    m_max_force = double_val;
    return(true);
  } else if ((param == "display_force_sources")) {
    return(setBooleanOnString(m_display_force_sources, val));
  } else if ((param == "display_total_force")) {
    return(setBooleanOnString(m_display_total_force, val));
  } else if ((param == "display_partial_forces")) {
    return(setBooleanOnString(m_display_partial_forces, val));
  } else if ((param == "display_statistics")) {
    return(setBooleanOnString(m_display_statistics, val));
  } else if ((param == "display_drift")) {
    return(setBooleanOnString(m_display_drift, val));
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_DriftingForces::onSetParamComplete()
{
  // Statistics
  m_statistics_label.set_label(m_us_name + "_stats");
  m_statistics_label.set_active(m_display_statistics);

  // Drift vector
  m_drift_vector.set_label(m_us_name + "_driftvec");
  m_drift_vector.set_active(m_display_drift);
  m_drift_vector.set_color("label", "invisible");

  // Total force vector
  m_rel_summed_force_vector.set_label(m_us_name + "_sum_force");
  m_rel_summed_force_vector.set_active(m_display_total_force);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_DriftingForces::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_DriftingForces::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_DriftingForces::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_DriftingForces::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_DriftingForces::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_DriftingForces::onRunToIdleState()
{
  m_statistics_label.set_active(false);
  m_drift_vector.set_active(false);
  postMessage("VIEW_POINT", m_statistics_label.get_spec());
  postMessage("VIEW_VECTOR", m_drift_vector.get_spec());
  postErasablePoints();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_DriftingForces::onRunState()
{
  IvPFunction *ipf = 0;

  postViewablePoints();

//  // Perform all calculations relative to ownship (i.e. consider ownship at origin)
//  double curr_dist_to_target = hypot(m_relative_target.x(), m_relative_target.y());
//  m_angle_to_target = relAng(0., 0., m_relative_target.x(), m_relative_target.y());
//  postMessage("DRIFTING_TARGET_DISTANCE", m_dist_to_target);
//  postMessage("DRIFTING_TARGET_HEADING", m_angle_to_target);
//  postMessage("DRIFTING_TARGET_RELATIVE_X", m_relative_target.x());
//  postMessage("DRIFTING_TARGET_RELATIVE_Y", m_relative_target.y());
//  postMessage("DRIFTING_TARGET_ABSOLUTE_X", m_absolute_target.x());
//  postMessage("DRIFTING_TARGET_ABSOLUTE_Y", m_absolute_target.y());
//
//  if (curr_dist_to_target > m_capture_radius) {           // outside capture radius
//    if (curr_dist_to_target < m_slip_radius) {            // inside slip radius
//      if (curr_dist_to_target > m_dist_to_target) {       // ownship moving away from target, so proxy for capture
//        ipf = IvPFunctionInsideCaptureRadius(m_angle_to_target);
//      } else {                                            // ownship moving toward target, so proxy for within drifting circle
//        ipf = IvPFunctionInsideDriftingRadius(m_angle_to_target, curr_dist_to_target);
//      }
//    } else if(curr_dist_to_target < m_drifting_radius) {  // outside slip radius but inside drifting radius
//      ipf = IvPFunctionInsideDriftingRadius(m_angle_to_target, curr_dist_to_target);
//    } else {                                              // outside drifting radius
//      ipf = IvPFunctionOutsideDriftingRadius(m_angle_to_target);
//    }
//  } else {                                                // inside capture radius
//    ipf = IvPFunctionInsideCaptureRadius(m_angle_to_target);
//  }
//
//  if (curr_dist_to_target < m_capture_radius && !m_drifting) {  // inside capture, and not drifting, so switch to drifting
//    m_drifting = true;
//    decreaseDriftingTargetsCauseThrusting();
//  } else if ((curr_dist_to_target < m_slip_radius) && (curr_dist_to_target > m_dist_to_target) && !m_drifting) {  // inside slip, moving away, and not drifting, so switch to drifting
//    m_drifting = true;
//    decreaseDriftingTargetsCauseThrusting();
//  } else if (curr_dist_to_target > m_drifting_radius && m_drifting) { // outside drifting, so switch to thrusting
//    m_drifting = false;
//    increaseDriftingTargetsCauseThrusting();
//  }
//
//  m_dist_to_target = curr_dist_to_target;
//
//  updateStatistics();
//
//  if(ipf) {
//    if(m_drifting) {
//      ipf->setPWT(0.);
//    } else {
//      ipf->setPWT(m_priority_wt);
//    }
//  }

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: updateInfoIn()
//   Purpose: update the info from MOOSDB vars.

bool BHV_DriftingForces::updateInfoIn()
{
  bool ok1, ok2;

  m_nav_x = getBufferDoubleVal("NAV_X", ok1);
  m_nav_y = getBufferDoubleVal("NAV_Y", ok2);
  if (!ok1 || !ok2) {
    postEMessage("No NAV_X or NAV_Y info in info_buffer.");
    return(false);
  }

  m_nav_heading = getBufferDoubleVal("NAV_HEADING", ok1);
  if (!ok1) {
    postEMessage("No NAV_HEADING info in info_buffer.");
    return(false);
  }

  m_nav_depth = getBufferDoubleVal("NAV_DEPTH", ok1);
  if (!ok1) {
    postEMessage("No NAV_DEPTH info in info_buffer.");
    return(false);
  }

  m_energy = getBufferDoubleVal("CONSO_ALL", ok1);
  if (!ok1) {
    //postWMessage("No CONSO_ALL (energy) info in info_buffer");
  }

  m_drift_x = getBufferDoubleVal("DRIFT_X", ok1);
  if (!ok1) {
    //postWMessage("No DRIFT_X info in info_buffer");
    //postMessage("DRIFT_X", 0.0);
  }

  m_drift_y = getBufferDoubleVal("DRIFT_Y", ok1);
  if (!ok1) {
    //postWMessage("No DRIFT_Y info in info_buffer");
    //postMessage("DRIFT_Y", 0.0);
  }

  return(true);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionInsideCaptureForce()
//   Purpose: Return IvP function when inside/below the capture force limit (begin drifting).

IvPFunction*  BHV_DriftingForces::IvPFunctionInsideCaptureForce(double heading)
{
//  postMessage("DRIFTING_TARGET_REGION", "inside capture radius");
//
//  // Heading
//  ZAIC_PEAK hdg_zaic(m_domain, "course");
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
//  hdg_zaic.setValueWrap(true);
//  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();
//
//  // Speed
//  ZAIC_PEAK spd_zaic(m_domain, "speed");
//  spd_zaic.setValueWrap(false);
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  spd_zaic.setParams(0., 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
//  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
//
//  // Coupling
//  OF_Coupler coupler;
//  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionInsideDriftingForce()
//   Purpose: Return IvP function when inside/below the drifting force but outside capture force.

IvPFunction*  BHV_DriftingForces::IvPFunctionInsideDriftingForce(double heading, double dist_from_target)
{
//  postMessage("DRIFTING_TARGET_REGION", "inside drifting radius");
//
//  // Heading
//  ZAIC_PEAK hdg_zaic(m_domain, "course");
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
//  hdg_zaic.setValueWrap(true);
//  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();
//
//  // Speed
//  ZAIC_PEAK spd_zaic(m_domain, "speed");
//  spd_zaic.setValueWrap(false);
//  double speed_ratio = (dist_from_target - m_capture_radius) / (m_drifting_radius - m_capture_radius);
//  double speed = m_speed*speed_ratio + m_end_speed_scale*(1-speed_ratio);
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  spd_zaic.setParams(speed,                                 // summit
//                     0.1 * m_speed,                         // pwidth
//                     0.3 * m_speed,                         // bwidth
//                     50.,                                   // delta
//                     0.,                                    // minutil
//                     100.);                                 // maxutil
//  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
//
//  // Coupling
//  OF_Coupler coupler;
//  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionOutsideDriftingForce()
//   Purpose: Return IvP function when outside/above the drifting force.

IvPFunction*  BHV_DriftingForces::IvPFunctionOutsideDriftingForce(double heading)
{
//  postMessage("DRIFTING_TARGET_REGION", "outside drifting radius");
//
//  // Heading
//  ZAIC_PEAK hdg_zaic(m_domain, "course");
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
//  hdg_zaic.setValueWrap(true);
//  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();
//
//  // Speed
//  ZAIC_PEAK spd_zaic(m_domain, "speed");
//  spd_zaic.setValueWrap(false);
//  // summit, pwidth, bwidth, delta, minutil, maxutil
//  spd_zaic.setParams(m_speed, 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
//  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
//
//  // Coupling
//  OF_Coupler coupler;
//  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display visuals.

void BHV_DriftingForces::postViewablePoints()
{
  if (m_display_total_force) {
    m_rel_summed_force_vector.setPosition(m_nav_x, m_nav_y);
    m_rel_summed_force_vector.set_active(true);
    postMessage("VIEW_VECTOR", m_rel_summed_force_vector.get_spec());
  }

  if (m_display_partial_forces) {
    int i = 0;
    ostringstream strs;
    for (list<XYVector>::iterator it = m_rel_forces.begin(); it != m_rel_forces.end(); ++it) {
      strs << m_us_name << "_force_" << i;
      it->set_label(strs.str());
      it->setPosition(m_nav_x, m_nav_y);
      strs.clear();
      strs.str("");
      it->set_active(true);
      it->set_color("label", "invisible");
      postMessage("VIEW_VECTOR", it->get_spec());
      i++;
    }
  }

  if (m_display_force_sources) {
    int i = 0;
    ostringstream strs;
    for (list<XYPoint>::iterator it = m_force_sources.begin(); it != m_force_sources.end(); ++it) {
      strs << m_us_name << "_source_" << i;
      it->set_label(strs.str());
      strs.clear();
      strs.str("");
      it->set_active(true);
      postMessage("VIEW_POINT", it->get_spec());
      i++;
    }
  }
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase visuals.

void BHV_DriftingForces::postErasablePoints()
{
  if (m_display_total_force) {
    m_rel_summed_force_vector.setPosition(m_nav_x, m_nav_y);
    m_rel_summed_force_vector.set_active(false);
    postMessage("VIEW_VECTOR", m_rel_summed_force_vector.get_spec());
  }

  if (m_display_partial_forces) {
    int i = 0;
    ostringstream strs;
    for (list<XYVector>::iterator it = m_rel_forces.begin(); it != m_rel_forces.end(); ++it) {
      strs << m_us_name << "_force_" << i;
      it->set_label(strs.str());
      strs.clear();
      strs.str("");
      it->set_active(false);
      postMessage("VIEW_VECTOR", it->get_spec());
      i++;
    }
  }

  if (m_display_force_sources) {
    int i = 0;
    ostringstream strs;
    for (list<XYPoint>::iterator it = m_force_sources.begin(); it != m_force_sources.end(); ++it) {
      it->set_color("vertex", "invisible");
      it->set_active(true);
      postMessage("VIEW_POINT", it->get_spec());
      i++;
    }
  }
}

//---------------------------------------------------------------
// Procedure: addRelativeForceSource()
//   Purpose: Add a force source to our list.

void BHV_DriftingForces::addRelativeForceSource(double x, double y, double weight)
{
  m_force_source.set_vertex(m_nav_x + x, m_nav_y + y);
  m_force_sources.push_back(m_force_source);

  relativeForceSourceToForce(x, y);
  m_rel_force_x = m_rel_force_x*weight;
  m_rel_force_y = m_rel_force_y*weight;

  m_rel_force.setVectorXY(m_rel_force_x, m_rel_force_y);
  m_rel_forces.push_front(m_rel_force);

  m_rel_summed_force_x += m_rel_force_x;
  m_rel_summed_force_y += m_rel_force_y;
  ostringstream strs;
  strs << " X " << m_rel_summed_force_x << " Y " << m_rel_summed_force_y << endl;
  postWMessage(strs.str());

  m_rel_summed_force_vector.setVectorXY(m_rel_summed_force_x*1000, m_rel_summed_force_y*1000);
}

//---------------------------------------------------------------
// Procedure: clearForceSources()
//   Purpose: Clear our force sources list.

void BHV_DriftingForces::clearForceSources()
{
  postWMessage("CLEARING!");
  postErasablePoints();
  m_force_source.clear();
  m_rel_forces.clear();
  m_rel_summed_force_x = 0.;
  m_rel_summed_force_y = 0.;
}

//---------------------------------------------------------------
// Procedure: relativeForceSourceToForce()
//   Purpose: Convert a relative force location to a force vector (note: we have different force-distance functions).

void BHV_DriftingForces::relativeForceSourceToForce(double x, double y)
{
  double dist = hypot(x, y);
  double ang = relAng(0., 0., x, y);
  double mag = vanDerWaals(1, 300, 300, 12, 6, dist);
  projectPoint(angle360(ang), mag, 0, 0, m_rel_force_x, m_rel_force_y);
}

//---------------------------------------------------------------
// Procedure: vanDerWaals()
//   Purpose: Van der Waals interatomic force model - given by E = eps*((A/r)^alpha - 2*(B/r)^beta) and F = eps*((A*alpha*(A/r)^(alpha - 1))/r^2 - (2*B*beta*(B/r)^(beta - 1))/r^2).

double BHV_DriftingForces::vanDerWaals(double eps, double A, double B, int alpha, int beta, double dist)
{
  return eps*((A*alpha*pow(A/dist,alpha-1))/pow(dist,2) - (2*B*beta*pow(B/dist,beta-1))/pow(dist,2));
}
