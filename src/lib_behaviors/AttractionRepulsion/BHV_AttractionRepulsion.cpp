/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsion.cpp                     */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_AttractionRepulsion.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_AttractionRepulsion::BHV_AttractionRepulsion(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingForce(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_AttractionRepulsion");

  // Initialize config variables
  m_contact_rangeout = 650.;
  m_display_targets = true;
  m_weight_targets = "";
  m_averaging = false;
  m_weight_averaging = false;

  // Initialize state variables
  m_total_force_x = 0.;
  m_total_force_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_weight_distance = true;
  m_weight_pingtime = true;
  m_total_weight = 0.;
  m_num_targets = 0;
  m_num_targets_prev = 0;
  m_warning_message.clear();
  m_warning_message.str("");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_AttractionRepulsion::setParam(string param, string val)
{
  // multiple inheritance is ugly, but other options are worse!
  if (BHV_DriftingForce::setParam(param, val))
    return true;
  if (BHV_AcousticPingPlanner::setParam(param, val))
    return true;

  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  bool non_neg_number = (isNumber(val) && (double_val >= 0));

  if ((param == "contact_rangeout") && non_neg_number) {
    m_contact_rangeout = double_val;
    return(true);
  } else if ((param == "display_targets")) {
    return(setBooleanOnString(m_display_targets, val));
  } else if ((param == "weight_targets")) {
    m_weight_targets = val;
    string pingtime = tokStringParse(m_weight_targets, "pingtime", ',', '=');
    string distance = tokStringParse(m_weight_targets, "distance", ',', '=');
    if ((pingtime == "") || (distance == "")) return false;
    if (pingtime == "true") {
      m_weight_pingtime = true;
    } else {
      m_weight_pingtime = false;
    }
    if (distance == "true") {
      m_weight_distance = true;
    } else {
      m_weight_distance = false;
    }
    return true;
  } else if ((param == "averaging")) {
    return(setBooleanOnString(m_averaging, val));
  } else if ((param == "weight_averaging")) {
    return(setBooleanOnString(m_weight_averaging, val));
  } else if ((param == "zero_crossing") && isNumber(val)) {
    m_zero_crossing = double_val;
    return(true);
  } else if ((param == "piece1_gradient") && isNumber(val)) {
    m_piece1_gradient = double_val;
    return(true);
  } else if ((param == "piece2_diff_x") && isNumber(val)) {
    m_piece2_diff_x = double_val;
    return(true);
  } else if ((param == "piece2_diff_y") && isNumber(val)) {
    m_piece2_diff_y = double_val;
    return(true);
  } else if ((param == "piece3_diff_x") && isNumber(val)) {
    m_piece3_diff_x = double_val;
    return(true);
  } else if ((param == "piece3_diff_y") && isNumber(val)) {
    m_piece3_diff_y = double_val;
    return(true);
  } else if ((param == "piece4_diff_x") && isNumber(val)) {
    m_piece4_diff_x = double_val;
    return(true);
  } else if ((param == "piece4_diff_y") && isNumber(val)) {
    m_piece4_diff_y = double_val;
    return(true);
  } else if ((param == "piece5_diff_x") && isNumber(val)) {
    m_piece5_diff_x = double_val;
    return(true);
  } else if ((param == "piece5_diff_y") && isNumber(val)) {
    m_piece5_diff_y = double_val;
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

void BHV_AttractionRepulsion::onSetParamComplete()
{
  BHV_DriftingForce::onSetParamComplete();
  BHV_AcousticPingPlanner::onSetParamComplete();

  // set random color for targets and circles
  int sum = 0;
  for (unsigned int i = 0; i < m_us_name.size(); i++) {
    sum += m_us_name[i];
  }
  srand(sum + time(NULL));
  double r, g, b;
  r = ((double)rand()/(double)RAND_MAX);
  g = ((double)rand()/(double)RAND_MAX);
  b = ((double)rand()/(double)RAND_MAX);
  ColorPack color(r,g,0);
  m_color = color;

//  m_warning_message << r << "," << g << "," << 0;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");

  m_abs_target.set_color("label", "invisible");
  m_abs_target.set_color("vertex", m_color);
  m_abs_target.set_vertex_size(3);

  m_abs_circle.set_color("label", "invisible");
  m_abs_circle.set_color("vertex", m_color);
  m_abs_circle.set_color("edge", m_color);
  m_abs_circle.set_color("fill", "invisible");
  m_abs_circle.setRad(m_zero_crossing/2);
  m_abs_circle.set_transparency(0.1);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_AttractionRepulsion::onHelmStart()
{
  BHV_DriftingForce::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_AttractionRepulsion::onIdleState()
{
  BHV_DriftingForce::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_AttractionRepulsion::onCompleteState()
{
  BHV_DriftingForce::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_AttractionRepulsion::postConfigStatus()
{
  BHV_DriftingForce::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_AttractionRepulsion::onIdleToRunState()
{
  BHV_DriftingForce::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_AttractionRepulsion::onRunToIdleState()
{
  BHV_DriftingForce::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_AttractionRepulsion::onRunState()
{
  if (!BHV_DriftingForce::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  /* Begin relative target calculations here - this behavior uses target point forces for formation keeping */
  m_total_force_x = 0.;
  m_total_force_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_total_weight = 0.;
  m_num_targets = 0;
  m_warning_message.clear();
  m_warning_message.str("");
  m_abs_targets.clear();

  // loop through all possible pairs of neighbours in the formation whitelist (don't need the plan, just list of neighbours for acoustics)
  for (map<string,PingContact>::iterator neighbour = m_contact_whitelist.begin(); neighbour != m_contact_whitelist.end(); ++neighbour) {  // loop through all neighbours in the whitelist
    if (neighbour->second.GetExpired()) {
      m_warning_message << neighbour->first << " neighbour has not been 'seen' acoustically for more than (specified) " << BHV_AcousticPingPlanner::m_contact_timeout << " seconds.";
      postWMessage(m_warning_message.str());
      m_warning_message.clear();
      m_warning_message.str("");
      continue;
    }

    // Relative positions of this neighbour from acoustic pings
    double x_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingForce::m_nav_x;
    double y_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingForce::m_nav_y;

    // Relative distance to this neighbour from acoustic pings
    double dist_rel_ping = hypot(x_rel_ping, y_rel_ping);
    double heading_rel_ping = relAng(0, 0, x_rel_ping, y_rel_ping);

    if (dist_rel_ping > m_contact_rangeout) {
      m_warning_message << neighbour->first << " neighbour is outside of rangeout distance " << m_contact_rangeout << ".";
      postWMessage(m_warning_message.str());
      m_warning_message.clear();
      m_warning_message.str("");
      continue;
    }

    // From the acoustic ping position, calculate our relative force
    double force_x, force_y;
    attractionRepulsionAtomic(dist_rel_ping, heading_rel_ping, force_x, force_y);

    // Calculate weighting according to the age of the pings of this neighbour
    double pingtime = neighbour->second.GetTimeLapse();
    if (m_weight_pingtime) {
      m_pingtime_weight = 1-(pingtime/BHV_AcousticPingPlanner::m_contact_timeout);
    } else {
      m_pingtime_weight = 1;
    }
    if (m_pingtime_weight < 0) {  //should not occur, since we break the loop if a contact has timed out
      m_pingtime_weight = 0.;
    }

    // Calculate weighting according to the distance of this neighbour
    if (m_weight_distance) {
      m_distance_weight = 1-(dist_rel_ping/m_contact_rangeout);
    } else {
      m_distance_weight = 1;
    }

    // Add this calculated force position to the total force, and weigh accordingly
    force_x = m_pingtime_weight*m_distance_weight*force_x;
    force_y = m_pingtime_weight*m_distance_weight*force_y;
    m_total_force_x += force_x;
    m_total_force_y += force_y;
    if (m_weight_averaging) {
      m_total_weight += m_pingtime_weight*m_distance_weight;
    } else {
      m_total_weight += 1;
    }
    m_num_targets++;

    if (m_display_targets) {
      // target point
      m_warning_message << m_us_name << "_ar_" << m_num_targets;
      m_abs_target.set_vertex(x_rel_ping + BHV_DriftingForce::m_nav_x, y_rel_ping + BHV_DriftingForce::m_nav_y);
      m_abs_target.set_color("vertex", m_color);
      m_abs_target.set_label(m_warning_message.str());
      postMessage("VIEW_POINT", m_abs_target.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
      // target circle
      m_warning_message << m_us_name << "_ar_cir_" << m_num_targets;
      m_abs_circle.setX(x_rel_ping + BHV_DriftingForce::m_nav_x);
      m_abs_circle.setY(y_rel_ping + BHV_DriftingForce::m_nav_y);
      m_abs_circle.set_color("vertex", m_color);
      m_abs_circle.set_color("edge", m_color);
      m_abs_circle.set_label(m_warning_message.str());
      postMessage("VIEW_CIRCLE", m_abs_circle.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
    }
  }

  m_abs_circle.setX(BHV_DriftingForce::m_nav_x);
  m_abs_circle.setY(BHV_DriftingForce::m_nav_y);
  m_abs_circle.setRad(m_zero_crossing/2);
  m_abs_circle.set_label(m_us_name + "_atom");
  postMessage("VIEW_CIRCLE", m_abs_circle.get_spec());

  if (m_display_targets && (m_num_targets < m_num_targets_prev)) {
    // remove previously drawn points and circles that are no longer valid
    int diff = m_num_targets_prev - m_num_targets;
    for (unsigned int i = 1; i <= diff; i++) {
      // target point
      m_warning_message << m_us_name << "_ar_" << m_num_targets + i;
      m_abs_target.set_vertex(0, 0);
      m_abs_target.set_color("vertex", "invisible");
      m_abs_target.set_label(m_warning_message.str());
      postMessage("VIEW_POINT", m_abs_target.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
      // target circle
      m_warning_message << m_us_name << "_ar_cir_" << m_num_targets + i;
      m_abs_circle.setX(0);
      m_abs_circle.setY(0);
      m_abs_circle.set_color("vertex", "invisible");
      m_abs_circle.set_color("edge", "invisible");
      m_abs_circle.set_label(m_warning_message.str());
      postMessage("VIEW_CIRCLE", m_abs_circle.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
    }
  }

  postMessage("ATTRACTION_REPULSION_TOTAL_WEIGHT", m_total_weight);
  postMessage("ATTRACTION_REPULSION_NUM_TARGETS", m_num_targets);
  if (m_averaging) {
    m_total_force_x = m_total_force_x/m_total_weight;
    m_total_force_y = m_total_force_y/m_total_weight;
  }
  m_num_targets_prev = m_num_targets;
  /* End relative target calculations here */

  if (m_num_targets != 0) { // no neighbours exist - do not set a new force
    BHV_DriftingForce::setForce(m_total_force_x, m_total_force_y);
    // note: if no neighbours, what should we do? should we stay at the previous force? or should we return null and allow free drifting?
  }

  return BHV_DriftingForce::onRunState();
}

//---------------------------------------------------------------
// Procedure: attractionRepulsionAtomic()
//   Purpose: Attraction/repulsion model based on atomic force.

void BHV_AttractionRepulsion::attractionRepulsionAtomic(double dist, double heading, double& force_x, double& force_y)
{
  double a, b, x, y, force, force_heading;
  int x_pow, y_pow, diff_pow;
  x_pow = 1;
  y_pow = 2;
  diff_pow = y_pow - x_pow;
  x = pow(dist,x_pow);
  y = pow((dist - m_zero_crossing),y_pow);
  a = 1.0;
  b = 1000.0;
  force = -dist*(a - b/y);
  if (dist < 300) force = 1000;
  if (force < 0) {
    force = -force;
    force_heading = heading;
  } else {
    force = force;
    force_heading = angle360(heading + 180.0);
  }
  projectPoint(force_heading, force, 0., 0., force_x, force_y);
}

//---------------------------------------------------------------
// Procedure: attractionRepulsionPiecewiseLinear()
//   Purpose: Attraction/repulsion model based on a piecewise linear model.

void BHV_AttractionRepulsion::attractionRepulsionPiecewiseLinear(double dist, double heading, double& force_x, double& force_y)
{
  double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, force, force_heading;

  x2 = m_zero_crossing + m_piece2_diff_x;
  y2 = 0 + m_piece2_diff_y;

  x1 = 0;
  y1 = y2 - m_piece1_gradient*x2;

  x3 = m_zero_crossing + m_piece3_diff_x;
  y3 = 0 + m_piece3_diff_y;

  x4 = x3 + m_piece4_diff_x;
  y4 = y3 + m_piece4_diff_y;

  x5 = x4 + m_piece5_diff_x;
  y5 = y4 + m_piece5_diff_y;

//  m_warning_message << x1 << "," << y1 << "," << x2 << "," << y2 << "," << x3 << "," << y3 << "," << x4 << "," << y4 << "," << x5 << "," << y5;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");

  if (dist >= x1 && dist < x2) {
    force = y1 + (y2 - y1)*(dist - x1)/(x2 - x1);
  } else if (dist >= x2 && dist < x3) {
    force = y2 + (y3 - y2)*(dist - x2)/(x3 - x2);
  } else if (dist >= x3 && dist < x4) {
    force = y3 + (y4 - y3)*(dist - x3)/(x4 - x3);
  } else if (dist >= x4 && dist < x5) {
    force = y4 + (y5 - y4)*(dist - x4)/(x5 - x4);
  } else {
    force = 0;
  }

  if (force < 0) {
    force = -force;
    force_heading = heading;
  } else {
    force = force;
    force_heading = angle360(heading + 180.0);
  }
  projectPoint(force_heading, force, 0., 0., force_x, force_y);
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display target points.

void BHV_AttractionRepulsion::postViewablePoints()
{
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_AttractionRepulsion::postErasablePoints()
{
}
