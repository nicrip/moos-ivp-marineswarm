/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Modification of Formation_Behaviour by Simon Rohou */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTarget.cpp                          */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <limits>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingTarget.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"

using namespace std;

int BHV_DriftingTarget::_num_drifting_targets = 0;
int BHV_DriftingTarget::_num_drifting_targets_cause_thrusting = 0;
double BHV_DriftingTarget::_time_thrusting_startclock = MOOSTime();
double BHV_DriftingTarget::_time_thrusting = 0;
double BHV_DriftingTarget::_time_drifting_startclock = MOOSTime();
double BHV_DriftingTarget::_time_drifting = 0;

//---------------------------------------------------------------
// Constructor

BHV_DriftingTarget::BHV_DriftingTarget(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_DriftingTarget");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Initialize config variables
  m_speed = 1.;
  m_capture_radius = 3.;
  m_slip_radius = 10.;
  m_drifting_radius = 20.;
  m_targets_filter_size = 1;
  m_display_filtered_target = true;
  m_display_unfiltered_targets = false;
  m_display_radii = true;
  m_display_statistics = true;
  m_display_drift = true;
  m_end_speed_scale = 0.5;

  // Initialize state variables
  m_nav_heading = 0.;
  m_nav_depth = 0.;
  m_nav_x = 0.;
  m_nav_y = 0.;
  m_energy = 0.;
  m_dist_to_target = numeric_limits<double>::infinity();
  m_drift_x = 0.;
  m_drift_y = 0.;
  m_relative_target.set_vertex(0., 0.);
  m_absolute_target.set_vertex(0., 0.);
  m_drifting = true;

  //BHV_DriftingTarget::increaseDriftingTargetsCauseThrusting();
  _num_drifting_targets++;
  postIntMessage("DRIFTING_TARGET_NUM_BHV", _num_drifting_targets);

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_DEPTH, NAV_HEADING, CONSO_ALL, DRIFT_X, DRIFT_Y");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingTarget::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  bool non_neg_number = (isNumber(val) && (double_val >= 0));

  if ((param == "capture_radius") && non_neg_number) {
    m_capture_radius = double_val;
    return(true);
  } else if ((param == "slip_radius") && non_neg_number) {
    m_slip_radius = double_val;
    return(true);
  } else if ((param == "drifting_radius") && non_neg_number) {
    m_drifting_radius = double_val;
    return(true);
  } else if ((param == "speed") && non_neg_number) {
    m_speed = double_val;
    return(true);
  } else if ((param == "display_filtered_target")) {
    return(setBooleanOnString(m_display_filtered_target, val));
  } else if ((param == "display_unfiltered_targets")) {
    return(setBooleanOnString(m_display_unfiltered_targets, val));
  } else if ((param == "display_radii")) {
    return(setBooleanOnString(m_display_radii, val));
  } else if ((param == "display_statistics")) {
    return(setBooleanOnString(m_display_statistics, val));
  } else if ((param == "display_drift")) {
    return(setBooleanOnString(m_display_drift, val));
  } else if ((param == "end_speed_scale") && non_neg_number) {
    m_end_speed_scale = double_val;
    if (m_end_speed_scale > 1.0)
      m_end_speed_scale = 1.0;
    return(true);
  } else if ((param == "targets_filter_size") && non_neg_number) {
    m_targets_filter_size = (int)double_val;
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

void BHV_DriftingTarget::onSetParamComplete()
{
  // Statistics
  m_statistics_label.set_label(m_us_name + "_stats");
  m_statistics_label.set_active(m_display_statistics);

  // Drift vector
  m_drift_vector.set_label(m_us_name + "_driftvec");
  m_drift_vector.set_active(m_display_drift);
  m_drift_vector.set_color("label", "invisible");

  // Target
  m_absolute_target.set_label(m_us_name + "_target");
  m_absolute_target.set_active("false");
  m_absolute_target.set_color("label", "cyan");
  m_absolute_target.set_color("vertex", "cyan");
  m_absolute_target.set_vertex_size(5);

  // Capture radius
  m_capture_radius_circle.set_label(m_us_name + "_capture");
  m_capture_radius_circle.set_active("false");
  m_capture_radius_circle.set_color("label", "red");
  m_capture_radius_circle.set_color("edge", "red");
  m_capture_radius_circle.set_color("fill", "red");
  m_capture_radius_circle.setRad(m_capture_radius);
  m_capture_radius_circle.set_vertex_size(2);
  m_capture_radius_circle.set_edge_size(1);
  m_capture_radius_circle.set_transparency(0.1);

  // Slip radius
  m_slip_radius_circle.set_label(m_us_name + "_slip");
  m_slip_radius_circle.set_active("false");
  m_slip_radius_circle.set_color("label", "orange");
  m_slip_radius_circle.set_color("edge", "orange");
  m_slip_radius_circle.set_color("fill", "orange");
  m_slip_radius_circle.setRad(m_slip_radius);
  m_slip_radius_circle.set_vertex_size(2);
  m_slip_radius_circle.set_edge_size(1);
  m_slip_radius_circle.set_transparency(0.1);

  // Drifting radius
  m_drifting_radius_circle.set_label(m_us_name + "_drift");
  m_drifting_radius_circle.set_active("false");
  m_drifting_radius_circle.set_color("label", "yellow");
  m_drifting_radius_circle.set_color("edge", "yellow");
  m_drifting_radius_circle.set_color("fill", "yellow");
  m_drifting_radius_circle.setRad(m_drifting_radius);
  m_drifting_radius_circle.set_vertex_size(2);
  m_drifting_radius_circle.set_edge_size(1);
  m_drifting_radius_circle.set_transparency(0.1);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_DriftingTarget::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_DriftingTarget::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_DriftingTarget::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_DriftingTarget::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_DriftingTarget::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_DriftingTarget::onRunToIdleState()
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

IvPFunction* BHV_DriftingTarget::onRunState()
{
  IvPFunction *ipf = 0;

  postViewablePoints();

  // Perform all calculations relative to ownship (i.e. consider ownship at origin)
  double curr_dist_to_target = hypot(m_relative_target.x(), m_relative_target.y());
  m_angle_to_target = relAng(0., 0., m_relative_target.x(), m_relative_target.y());
  postMessage("DRIFTING_TARGET_DISTANCE", m_dist_to_target);
  postMessage("DRIFTING_TARGET_HEADING", m_angle_to_target);
  postMessage("DRIFTING_TARGET_RELATIVE_X", m_relative_target.x());
  postMessage("DRIFTING_TARGET_RELATIVE_Y", m_relative_target.y());
  postMessage("DRIFTING_TARGET_ABSOLUTE_X", m_absolute_target.x());
  postMessage("DRIFTING_TARGET_ABSOLUTE_Y", m_absolute_target.y());

  if (curr_dist_to_target > m_capture_radius) {           // outside capture radius
    if (curr_dist_to_target < m_slip_radius) {            // inside slip radius
      if (curr_dist_to_target > m_dist_to_target) {       // ownship moving away from target, so proxy for capture
        ipf = IvPFunctionInsideCaptureRadius(m_angle_to_target);
      } else {                                            // ownship moving toward target, so proxy for within drifting circle
        ipf = IvPFunctionInsideDriftingRadius(m_angle_to_target, curr_dist_to_target);
      }
    } else if(curr_dist_to_target < m_drifting_radius) {  // outside slip radius but inside drifting radius
      ipf = IvPFunctionInsideDriftingRadius(m_angle_to_target, curr_dist_to_target);
    } else {                                              // outside drifting radius
      ipf = IvPFunctionOutsideDriftingRadius(m_angle_to_target);
    }
  } else {                                                // inside capture radius
    ipf = IvPFunctionInsideCaptureRadius(m_angle_to_target);
  }

  if (curr_dist_to_target < m_capture_radius && !m_drifting) {  // inside capture, and not drifting, so switch to drifting
    m_drifting = true;
    decreaseDriftingTargetsCauseThrusting();
  } else if ((curr_dist_to_target < m_slip_radius) && (curr_dist_to_target > m_dist_to_target) && !m_drifting) {  // inside slip, moving away, and not drifting, so switch to drifting
    m_drifting = true;
    decreaseDriftingTargetsCauseThrusting();
  } else if (curr_dist_to_target > m_drifting_radius && m_drifting) { // outside drifting, so switch to thrusting
    m_drifting = false;
    increaseDriftingTargetsCauseThrusting();
  }

  m_dist_to_target = curr_dist_to_target;

  updateStatistics();

  if(ipf) {
    if(m_drifting) {
      ipf->setPWT(0.);
    } else {
      ipf->setPWT(m_priority_wt);
    }
  }

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: updateInfoIn()
//   Purpose: update the info from MOOSDB vars.

bool BHV_DriftingTarget::updateInfoIn()
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
// Procedure: IvPFunctionInsideCaptureRadius()
//   Purpose: Return IvP function when inside the capture radius (begin drifting).

IvPFunction*  BHV_DriftingTarget::IvPFunctionInsideCaptureRadius(double heading)
{
  postMessage("DRIFTING_TARGET_REGION", "inside capture radius");

  // Heading
  ZAIC_PEAK hdg_zaic(m_domain, "course");
  // summit, pwidth, bwidth, delta, minutil, maxutil
  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
  hdg_zaic.setValueWrap(true);
  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setValueWrap(false);
  // summit, pwidth, bwidth, delta, minutil, maxutil
  spd_zaic.setParams(0., 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
  OF_Coupler coupler;
  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionInsideDriftingRadius()
//   Purpose: Return IvP function when inside the drifting radius but outside capture radius (begin slowdown).

IvPFunction*  BHV_DriftingTarget::IvPFunctionInsideDriftingRadius(double heading, double dist_from_target)
{
  postMessage("DRIFTING_TARGET_REGION", "inside drifting radius");

  // Heading
  ZAIC_PEAK hdg_zaic(m_domain, "course");
  // summit, pwidth, bwidth, delta, minutil, maxutil
  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
  hdg_zaic.setValueWrap(true);
  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setValueWrap(false);
  double speed_ratio = (dist_from_target - m_capture_radius) / (m_drifting_radius - m_capture_radius);
  double speed = m_speed*speed_ratio + m_end_speed_scale*(1-speed_ratio);
  // summit, pwidth, bwidth, delta, minutil, maxutil
  spd_zaic.setParams(speed,                                 // summit
                     0.1 * m_speed,                         // pwidth
                     0.3 * m_speed,                         // bwidth
                     50.,                                   // delta
                     0.,                                    // minutil
                     100.);                                 // maxutil
  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
  OF_Coupler coupler;
  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionOutsideDriftingRadius()
//   Purpose: Return IvP function when outside the drifting radius (max speed).

IvPFunction*  BHV_DriftingTarget::IvPFunctionOutsideDriftingRadius(double heading)
{
  postMessage("DRIFTING_TARGET_REGION", "outside drifting radius");

  // Heading
  ZAIC_PEAK hdg_zaic(m_domain, "course");
  // summit, pwidth, bwidth, delta, minutil, maxutil
  hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
  hdg_zaic.setValueWrap(true);
  IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setValueWrap(false);
  // summit, pwidth, bwidth, delta, minutil, maxutil
  spd_zaic.setParams(m_speed, 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
  OF_Coupler coupler;
  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display target circles.

void BHV_DriftingTarget::postViewablePoints()
{
  //m_absolute_target.set_vertex(m_nav_x + m_relative_target.x(), m_nav_y + m_relative_target.y(), -m_nav_depth + m_relative_target.z());
  m_relative_target.set_vertex(m_absolute_target.x() - m_nav_x, m_absolute_target.y() - m_nav_y);

  if (m_display_filtered_target) {
    //m_absolute_target.set_active(!m_drifting);
    m_absolute_target.set_active(true);
    postMessage("VIEW_POINT", m_absolute_target.get_spec());
  }

  if (m_display_unfiltered_targets) {
    updateViewUnfilteredPoints(!m_drifting);
    //updateViewUnfilteredPoints(true);
  }

  if (m_display_radii) {
    m_capture_radius_circle.setX(m_absolute_target.x());
    m_capture_radius_circle.setY(m_absolute_target.y());
    m_capture_radius_circle.set_active(!m_drifting);
    postMessage("VIEW_CIRCLE", m_capture_radius_circle.get_spec());
    m_slip_radius_circle.setX(m_absolute_target.x());
    m_slip_radius_circle.setY(m_absolute_target.y());
    m_slip_radius_circle.set_active(!m_drifting);
    postMessage("VIEW_CIRCLE", m_slip_radius_circle.get_spec());
    m_drifting_radius_circle.setX(m_absolute_target.x());
    m_drifting_radius_circle.setY(m_absolute_target.y());
    m_drifting_radius_circle.set_active(true);
    if (m_drifting) {
      m_drifting_radius_circle.set_color("label", "khaki");
      m_drifting_radius_circle.set_color("edge", "khaki");
      m_drifting_radius_circle.set_color("fill", "khaki");
    } else {
      m_drifting_radius_circle.set_color("label", "yellow");
      m_drifting_radius_circle.set_color("edge", "yellow");
      m_drifting_radius_circle.set_color("fill", "yellow");
    }
    postMessage("VIEW_CIRCLE", m_drifting_radius_circle.get_spec());
  }
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target circles.

void BHV_DriftingTarget::postErasablePoints()
{
  //m_absolute_target.set_vertex(m_nav_x + m_relative_target.x(), m_nav_y + m_relative_target.y(), -m_nav_depth + m_relative_target.z());
  m_relative_target.set_vertex(m_absolute_target.x() - m_nav_x, m_absolute_target.y() - m_nav_y);

  if (m_display_filtered_target) {
    m_absolute_target.set_active(false);
    postMessage("VIEW_POINT", m_absolute_target.get_spec());
  }

  if (m_display_unfiltered_targets) {
    updateViewUnfilteredPoints(false);
  }

  if (m_display_radii) {
    m_capture_radius_circle.setX(m_absolute_target.x());
    m_capture_radius_circle.setY(m_absolute_target.y());
    m_capture_radius_circle.set_active(false);
    postMessage("VIEW_CIRCLE", m_capture_radius_circle.get_spec());
    m_slip_radius_circle.setX(m_absolute_target.x());
    m_slip_radius_circle.setY(m_absolute_target.y());
    m_slip_radius_circle.set_active(false);
    postMessage("VIEW_CIRCLE", m_slip_radius_circle.get_spec());
    m_drifting_radius_circle.setX(m_absolute_target.x());
    m_drifting_radius_circle.setY(m_absolute_target.y());
    m_drifting_radius_circle.set_active(false);
    if (m_drifting) {
      m_drifting_radius_circle.set_color("label", "khaki");
      m_drifting_radius_circle.set_color("edge", "khaki");
      m_drifting_radius_circle.set_color("fill", "khaki");
    } else {
      m_drifting_radius_circle.set_color("label", "yellow");
      m_drifting_radius_circle.set_color("edge", "yellow");
      m_drifting_radius_circle.set_color("fill", "yellow");
    }
    postMessage("VIEW_CIRCLE", m_drifting_radius_circle.get_spec());
  }
}

//---------------------------------------------------------------
// Procedure: updateViewUnfilteredPoints()
//   Purpose: Update view on unfiltered targetpoints.

void BHV_DriftingTarget::updateViewUnfilteredPoints(bool display)
{
  int i = 0;
  ostringstream strs;
  for (list<XYPoint>::iterator target = m_targets.begin(); target != m_targets.end(); ++target) {
    strs << m_us_name << "_tgt_" << i;
    target->set_label(strs.str());
    strs.clear();
    strs.str("");
    target->set_active(display);
    target->set_color("label", "invisible");
    //target->set_vertex(target->x()+m_nav_x, target->y()+m_nav_y, target->z());
    postMessage("VIEW_POINT", target->get_spec());
    //target->set_vertex(target->x()-m_nav_x, target->y()-m_nav_y, target->z());
    i++;
  }
}

//---------------------------------------------------------------
// Procedure: addRelativeTargetToFilter()
//   Purpose: Add a relative target to the filter, then perform filtering.

void BHV_DriftingTarget::addRelativeTargetToFilter(double x, double y)
{
  m_relative_target.set_vertex(x, y);
  m_absolute_target.set_vertex(m_nav_x + x, m_nav_y + y);
  filterCentroidTargets();
}

//---------------------------------------------------------------
// Procedure: filterCentroidTargets()
//   Purpose: Perform simple averaging of multiple targetpoints (to remove noise).

void BHV_DriftingTarget::filterCentroidTargets()
{
  m_targets.push_front(m_absolute_target);

  if (m_targets.size() > m_targets_filter_size)
    m_targets.pop_back();

  double x = 0.;
  double y = 0.;

  ostringstream strs;
  for (list<XYPoint>::iterator target = m_targets.begin(); target != m_targets.end(); ++target) {
    strs << "X: " << target->x() << " Y: " << target->y() << " | ";
    x += target->x();
    y += target->y();
  }

  postMessage("DRIFTING_TARGET_UNFILTERED_LIST", strs.str());
  x /= m_targets.size();
  y /= m_targets.size();

  m_absolute_target.set_vertex(x, y);
  m_relative_target.set_vertex(x - m_nav_x, y - m_nav_y);
}

//---------------------------------------------------------------
// Procedure: increaseDriftingTargetsCauseThrusting()
//   Purpose: Count number of driftingTarget behaviours that cause thrusting, and check if new target has appeared.

void BHV_DriftingTarget::increaseDriftingTargetsCauseThrusting()
{
  // no active targets, but we now have one, so restart thrusting clock
  if (_num_drifting_targets_cause_thrusting == 0) {
    _time_thrusting_startclock = MOOSTime();
    _time_drifting += MOOSTime() - _time_drifting_startclock;
  }

  _num_drifting_targets_cause_thrusting++;
}

//---------------------------------------------------------------
// Procedure: decreaseDriftingTargetsCauseThrusting()
//   Purpose: Count number of driftingTarget behaviours cause thrusting, and check if all targets gone.

void BHV_DriftingTarget::decreaseDriftingTargetsCauseThrusting()
{
  _num_drifting_targets_cause_thrusting--;

  // no more active targets, so we are now drifting, so restart drifting clock
  if (_num_drifting_targets_cause_thrusting == 0) {
    _time_drifting_startclock = MOOSTime();
    _time_thrusting += MOOSTime() - _time_thrusting_startclock;
  }
}

//---------------------------------------------------------------
// Procedure: getTimeThrusting()
//   Purpose: Calculate current amount of time used for thrusting.

double BHV_DriftingTarget::getTimeThrusting()
{
  if (_num_drifting_targets_cause_thrusting == 0) {
    return _time_thrusting;
  } else {
    return _time_thrusting + MOOSTime() - _time_thrusting_startclock; // currently thrusting, so get most current time
  }
}

//---------------------------------------------------------------
// Procedure: getTimeDrifting()
//   Purpose: Calculate current amount of time used for drifting.

double BHV_DriftingTarget::getTimeDrifting()
{
  if (_num_drifting_targets_cause_thrusting == 0) {
    return _time_drifting + MOOSTime() - _time_drifting_startclock; // currently drifting, so get most current time
  } else {
    return _time_drifting;
  }
}

//---------------------------------------------------------------
// Procedure: updateStatistics()
//   Purpose: Update statistics of energy and drifting/thrust ratio and drift vector.

void BHV_DriftingTarget::updateStatistics()
{
  double time_thrusting = getTimeThrusting();
  double time_drifting = getTimeDrifting();
  double drift_ratio = 100. * time_drifting / (time_drifting + time_thrusting);
  postMessage("DRIFTING_TARGET_TIME_DRIFTING", time_drifting);
  postMessage("DRIFTING_TARGET_TIME_THRUSTING", time_thrusting);
  postMessage("DRIFTING_TARGET_DRIFTING_RATIO", drift_ratio);
  postMessage("DRIFTING_TARGET_TOTAL_TIME", time_drifting + time_thrusting);
  postIntMessage("DRIFTING_TARGET_NUM_THRUSTING", _num_drifting_targets_cause_thrusting);

  if (m_display_statistics) {
    m_statistics_label.set_vertex(m_nav_x, m_nav_y + 10);
    ostringstream strs;
    strs << round(drift_ratio) << "% | " << std::fixed << std::setprecision(2) << m_energy/3600. << "Wh";
    m_statistics_label.set_msg(strs.str());
    m_statistics_label.set_color("label", "ghostwhite");
    m_statistics_label.set_color("vertex", "transparent");
    postMessage("VIEW_POINT", m_statistics_label.get_spec());
  }

  if (m_display_drift) {
    double drift_x = 500*m_drift_x;
    double drift_y = 500*m_drift_y;
    m_drift_vector.setPosition(m_nav_x, m_nav_y);
    m_drift_vector.setVectorXY(drift_x, drift_y);
    m_drift_vector.setHeadSize(15);
    m_drift_vector.set_edge_size(2);
    postMessage("VIEW_VECTOR", m_drift_vector.get_spec());
  }
}
