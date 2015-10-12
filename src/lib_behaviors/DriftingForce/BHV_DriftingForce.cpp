/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForce.cpp                           */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <limits>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingForce.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"

using namespace std;

int BHV_DriftingForce::_num_drifting_targets = 0;
int BHV_DriftingForce::_num_drifting_targets_cause_thrusting = 0;
double BHV_DriftingForce::_time_thrusting_startclock = MOOSTime();
double BHV_DriftingForce::_time_thrusting = 0;
double BHV_DriftingForce::_time_drifting_startclock = MOOSTime();
double BHV_DriftingForce::_time_drifting = 0;

//---------------------------------------------------------------
// Constructor

BHV_DriftingForce::BHV_DriftingForce(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_DriftingForce");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Initialize config variables
  m_speed = 1.;
  m_max_force = 100.;
  m_capture_force = 3.;
  m_slip_force = 15.;
  m_drifting_force = 30.;
  m_display_force = true;
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
  m_force_x = 0.;
  m_force_y = 0.;

  _num_drifting_targets++;
  postIntMessage("DRIFTING_FORCE_NUM_BHV", _num_drifting_targets);

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_DEPTH, NAV_HEADING, CONSO_ALL, DRIFT_X, DRIFT_Y");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingForce::setParam(string param, string val)
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
  } else if ((param == "display_force")) {
    return(setBooleanOnString(m_display_force, val));
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

void BHV_DriftingForce::onSetParamComplete()
{
  // Statistics
  m_statistics_label.set_label(m_us_name + "_stats");
  m_statistics_label.set_active(m_display_statistics);

  // Drift vector
  m_drift_vector.set_label(m_us_name + "_driftvec");
  m_drift_vector.set_active(m_display_drift);
  m_drift_vector.set_color("label", "invisible");

  // Force vector
  m_force_vector.set_label(m_us_name + "_force");
  m_force_vector.set_color("label", "invisible");
  m_force_vector.set_active(m_display_force);
  m_force_vector.setHeadSize(7);
  m_force_vector.set_edge_size(1);
  m_force_vector.set_color("edge", "white");

  // Capture force vector
  m_capture_force_vector.set_label(m_us_name + "_capture_force");
  m_capture_force_vector.set_color("label", "invisible");
  m_capture_force_vector.set_active(m_display_force);
  m_capture_force_vector.setHeadSize(5);
  m_capture_force_vector.set_edge_size(3);
  m_capture_force_vector.set_color("edge", "red");
  m_capture_force_vector.set_color("vertex", "red");

  // Slip force vector
  m_slip_force_vector.set_label(m_us_name + "_slip_force");
  m_slip_force_vector.set_color("label", "invisible");
  m_slip_force_vector.set_active(m_display_force);
  m_slip_force_vector.setHeadSize(5);
  m_slip_force_vector.set_edge_size(3);
  m_slip_force_vector.set_color("edge", "orange");
  m_slip_force_vector.set_color("vertex", "orange");

  // Drifting force vector
  m_drifting_force_vector.set_label(m_us_name + "_drifting_force");
  m_drifting_force_vector.set_color("label", "invisible");
  m_drifting_force_vector.set_active(m_display_force);
  m_drifting_force_vector.setHeadSize(5);
  m_drifting_force_vector.set_edge_size(3);
  m_drifting_force_vector.set_color("edge", "yellow");
  m_drifting_force_vector.set_color("vertex", "yellow");

  // Normalize forces for speeds
  m_capture_speed = m_capture_force/m_max_force*m_speed;
  m_slip_speed = m_slip_force/m_max_force*m_speed;
  m_drifting_speed = m_drifting_force/m_max_force*m_speed;
  postMessage("DRIFTING_FORCE_CAPTURE_SPEED", m_capture_speed);
  postMessage("DRIFTING_FORCE_SLIP_SPEED", m_slip_speed);
  postMessage("DRIFTING_FORCE_DRIFTING_SPEED", m_drifting_speed);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_DriftingForce::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_DriftingForce::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_DriftingForce::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_DriftingForce::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_DriftingForce::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_DriftingForce::onRunToIdleState()
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

IvPFunction* BHV_DriftingForce::onRunState()
{
   IvPFunction *ipf = 0;

  postViewablePoints();

  // normalize force for speed and calculate angle for heading
  double curr_force_magnitude = hypot(m_force_x, m_force_y);
  if (curr_force_magnitude > m_max_force) curr_force_magnitude = m_max_force;
  m_force_speed = curr_force_magnitude/m_max_force*m_speed;
  m_force_heading = relAng(0., 0., m_force_x, m_force_y);
  postMessage("DRIFTING_FORCE_FORCE_X", m_force_x);
  postMessage("DRIFTING_FORCE_FORCE_Y", m_force_y);
  postMessage("DRIFTING_FORCE_FORCE_HEADING", m_force_heading);
  postMessage("DRIFTING_FORCE_FORCE_MAGNITUDE", curr_force_magnitude);
  postMessage("DRIFTING_FORCE_FORCE_SPEED", m_force_speed);

  if (curr_force_magnitude > m_capture_force) {             // outside capture force
    if (curr_force_magnitude < m_slip_force) {              // inside slip force
      if (curr_force_magnitude > m_force_magnitude) {       // ownship force increasing again, so proxy for capture
        ipf = IvPFunctionInsideCaptureForce(m_force_heading, m_force_speed);
      } else {                                              // ownship moving toward target, so proxy for within drifting circle
        ipf = IvPFunctionInsideDriftingForce(m_force_heading, m_force_speed);
      }
    } else if(curr_force_magnitude < m_drifting_force) {    // outside slip force but inside drifting force
      ipf = IvPFunctionInsideDriftingForce(m_force_heading, m_force_speed);
    } else {                                                // outside drifting force
      ipf = IvPFunctionOutsideDriftingForce(m_force_heading, m_force_speed);
    }
  } else {                                                  // inside capture force
    ipf = IvPFunctionInsideCaptureForce(m_force_heading, m_force_speed);
  }

  if (curr_force_magnitude < m_capture_force && !m_drifting) {  // inside capture, and not drifting, so switch to drifting
    m_drifting = true;
    decreaseDriftingTargetsCauseThrusting();
  } else if ((curr_force_magnitude < m_slip_force) && (curr_force_magnitude > m_force_magnitude) && !m_drifting) {  // inside slip, moving away, and not drifting, so switch to drifting
    m_drifting = true;
    decreaseDriftingTargetsCauseThrusting();
  } else if (curr_force_magnitude > m_drifting_force && m_drifting) { // outside drifting, so switch to thrusting
    m_drifting = false;
    increaseDriftingTargetsCauseThrusting();
  }

  m_force_magnitude = curr_force_magnitude;

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

bool BHV_DriftingForce::updateInfoIn()
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
//   Purpose: Return IvP function when inside the capture force (begin drifting).

IvPFunction*  BHV_DriftingForce::IvPFunctionInsideCaptureForce(double heading, double speed)
{
  postMessage("DRIFTING_FORCE_REGION", "inside capture force");

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
// Procedure: IvPFunctionInsideDriftingForce()
//   Purpose: Return IvP function when inside the drifting force but outside capture force (begin slowdown).

IvPFunction*  BHV_DriftingForce::IvPFunctionInsideDriftingForce(double heading, double speed)
{
  postMessage("DRIFTING_FORCE_REGION", "inside drifting force");

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
  spd_zaic.setParams(speed, 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
  OF_Coupler coupler;
  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: IvPFunctionOutsideDriftingForce()
//   Purpose: Return IvP function when outside the drifting force (max speed).

IvPFunction*  BHV_DriftingForce::IvPFunctionOutsideDriftingForce(double heading, double speed)
{
  postMessage("DRIFTING_FORCE_REGION", "outside drifting force");

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
  spd_zaic.setParams(speed, 0.1 * m_speed, 0.3 * m_speed, 50., 0., 100.);
  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
  OF_Coupler coupler;
  return coupler.couple(hdg_ipf, spd_ipf);
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display force vectors.

void BHV_DriftingForce::postViewablePoints()
{
  if (m_display_force) {
    double force_x;
    double force_y;
    projectPoint(m_force_heading, m_drifting_force, 0., 0., force_x, force_y);
    m_drifting_force_vector.setPosition(m_nav_x, m_nav_y);
    m_drifting_force_vector.setVectorXY(force_x, force_y);
    m_drifting_force_vector.setHeadSize(5);
    m_drifting_force_vector.set_edge_size(3);
    m_drifting_force_vector.set_active(true);
    if (m_drifting) {
      m_drifting_force_vector.set_color("edge", "khaki");
      m_drifting_force_vector.set_color("vertex", "khaki");
    } else {
      m_drifting_force_vector.set_color("edge", "yellow");
      m_drifting_force_vector.set_color("vertex", "yellow");
    }
    postMessage("VIEW_VECTOR", m_drifting_force_vector.get_spec());
    projectPoint(m_force_heading, m_slip_force, 0., 0., force_x, force_y);
    m_slip_force_vector.setPosition(m_nav_x, m_nav_y);
    m_slip_force_vector.setVectorXY(force_x, force_y);
    m_slip_force_vector.setHeadSize(5);
    m_slip_force_vector.set_edge_size(3);
    m_slip_force_vector.set_active(!m_drifting);
    postMessage("VIEW_VECTOR", m_slip_force_vector.get_spec());
    projectPoint(m_force_heading, m_capture_force, 0., 0., force_x, force_y);
    m_capture_force_vector.setPosition(m_nav_x, m_nav_y);
    m_capture_force_vector.setVectorXY(force_x, force_y);
    m_capture_force_vector.setHeadSize(5);
    m_capture_force_vector.set_edge_size(3);
    m_capture_force_vector.set_active(!m_drifting);
    postMessage("VIEW_VECTOR", m_capture_force_vector.get_spec());
    m_force_vector.setPosition(m_nav_x, m_nav_y);
    double magnitude = hypot(m_force_x, m_force_y);
    if (magnitude > 2*m_max_force) {
      projectPoint(m_force_heading, 2*m_max_force, 0., 0., force_x, force_y);
      m_force_vector.setVectorXY(force_x, force_y);
    } else {
      m_force_vector.setVectorXY(m_force_x, m_force_y);
    }
    m_force_vector.setHeadSize(7);
    m_force_vector.set_edge_size(1);
    m_force_vector.set_active(true);
    postMessage("VIEW_VECTOR", m_force_vector.get_spec());
  }
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase force vectors.

void BHV_DriftingForce::postErasablePoints()
{
  if (m_display_force) {
    double force_x;
    double force_y;
    projectPoint(m_force_heading, m_drifting_force, 0., 0., force_x, force_y);
    m_drifting_force_vector.setPosition(m_nav_x, m_nav_y);
    m_drifting_force_vector.setVectorXY(force_x, force_y);
    m_drifting_force_vector.setHeadSize(5);
    m_drifting_force_vector.set_edge_size(3);
    m_drifting_force_vector.set_active(false);
    if (m_drifting) {
      m_drifting_force_vector.set_color("edge", "khaki");
      m_drifting_force_vector.set_color("vertex", "khaki");
    } else {
      m_drifting_force_vector.set_color("edge", "yellow");
      m_drifting_force_vector.set_color("vertex", "yellow");
    }
    postMessage("VIEW_VECTOR", m_drifting_force_vector.get_spec());
    projectPoint(m_force_heading, m_slip_force, 0., 0., force_x, force_y);
    m_slip_force_vector.setPosition(m_nav_x, m_nav_y);
    m_slip_force_vector.setVectorXY(force_x, force_y);
    m_slip_force_vector.setHeadSize(5);
    m_slip_force_vector.set_edge_size(3);
    m_slip_force_vector.set_active(false);
    postMessage("VIEW_VECTOR", m_slip_force_vector.get_spec());
    projectPoint(m_force_heading, m_capture_force, 0., 0., force_x, force_y);
    m_capture_force_vector.setPosition(m_nav_x, m_nav_y);
    m_capture_force_vector.setVectorXY(force_x, force_y);
    m_capture_force_vector.setHeadSize(5);
    m_capture_force_vector.set_edge_size(3);
    m_capture_force_vector.set_active(false);
    postMessage("VIEW_VECTOR", m_capture_force_vector.get_spec());
    m_force_vector.setPosition(m_nav_x, m_nav_y);
    double magnitude = hypot(m_force_x, m_force_y);
    if (magnitude > 2*m_max_force) {
      projectPoint(m_force_heading, 2*m_max_force, 0., 0., force_x, force_y);
      m_force_vector.setVectorXY(force_x, force_y);
    } else {
      m_force_vector.setVectorXY(m_force_x, m_force_y);
    }
    m_force_vector.setHeadSize(7);
    m_force_vector.set_edge_size(1);
    m_force_vector.set_active(false);
    postMessage("VIEW_VECTOR", m_force_vector.get_spec());
  }
}

//---------------------------------------------------------------
// Procedure: setForce()
//   Purpose: Set the force.

void BHV_DriftingForce::setForce(double force_x, double force_y)
{
  m_force_x = force_x;
  m_force_y = force_y;
}

//---------------------------------------------------------------
// Procedure: increaseDriftingTargetsCauseThrusting()
//   Purpose: Count number of driftingTarget behaviours that cause thrusting, and check if new target has appeared.

void BHV_DriftingForce::increaseDriftingTargetsCauseThrusting()
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

void BHV_DriftingForce::decreaseDriftingTargetsCauseThrusting()
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

double BHV_DriftingForce::getTimeThrusting()
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

double BHV_DriftingForce::getTimeDrifting()
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

void BHV_DriftingForce::updateStatistics()
{
  double time_thrusting = getTimeThrusting();
  double time_drifting = getTimeDrifting();
  double drift_ratio = 100. * time_drifting / (time_drifting + time_thrusting);
  postMessage("DRIFTING_FORCE_TIME_DRIFTING", time_drifting);
  postMessage("DRIFTING_FORCE_TIME_THRUSTING", time_thrusting);
  postMessage("DRIFTING_FORCE_DRIFTING_RATIO", drift_ratio);
  postMessage("DRIFTING_FORCE_TOTAL_TIME", time_drifting + time_thrusting);
  postIntMessage("DRIFTING_FORCE_NUM_THRUSTING", _num_drifting_targets_cause_thrusting);

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
