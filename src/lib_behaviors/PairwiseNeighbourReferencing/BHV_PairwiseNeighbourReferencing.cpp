/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_PairwiseNeighbourReferencing.cpp            */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_PairwiseNeighbourReferencing.h"
#include "Geometry/IncrementalConvexHull.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_PairwiseNeighbourReferencing::BHV_PairwiseNeighbourReferencing(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_PairwiseNeighbourReferencing");

  // Initialize config variables
  m_contact_rangeout = 650.;
  m_display_unaveraged_targets = false;
  m_display_unaveraged_hull = false;
  m_weight_targets = "";
  m_weight_averaging = false;

  // Initialize state variables
  m_rel_target_x = 0.;
  m_rel_target_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_weight_distance = true;
  m_weight_pingtime = true;
  m_total_weight = 0.;
  m_num_target_pairs = 0;
  m_num_target_pairs_prev = 0;
  m_warning_message.clear();
  m_warning_message.str("");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_PairwiseNeighbourReferencing::setParam(string param, string val)
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
  bool non_neg_number = (isNumber(val) && (double_val >= 0));

  if ((param == "contact_rangeout") && non_neg_number) {
    m_contact_rangeout = double_val;
    return(true);
  } else if ((param == "display_unaveraged_targets")) {
    return(setBooleanOnString(m_display_unaveraged_targets, val));
  } else if ((param == "display_unaveraged_hull")) {
    return(setBooleanOnString(m_display_unaveraged_hull, val));
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
  } else if ((param == "weight_averaging")) {
    return(setBooleanOnString(m_weight_averaging, val));
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_PairwiseNeighbourReferencing::onSetParamComplete()
{
  BHV_DriftingTarget::onSetParamComplete();
  BHV_AcousticPingPlanner::onSetParamComplete();

  m_abs_target.set_color("label", "invisible");
  m_abs_target.set_color("vertex", "cyan");
  m_abs_target.set_vertex_size(3);

  m_abs_targets.set_color("label", "invisible");
  m_abs_targets.set_color("vertex", "cyan");
  m_abs_targets.set_color("edge", "cyan");
  m_abs_targets.set_color("fill", "white");
  m_abs_targets.set_transparency(0.1);
  m_abs_targets.set_vertex_size(3);
  m_abs_targets.set_label(m_us_name + "_pnr_tgts");
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_PairwiseNeighbourReferencing::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_PairwiseNeighbourReferencing::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_PairwiseNeighbourReferencing::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_PairwiseNeighbourReferencing::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_PairwiseNeighbourReferencing::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_PairwiseNeighbourReferencing::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_PairwiseNeighbourReferencing::onRunState()
{
  if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  /* Begin relative target calculations here - this behavior uses target points for formation keeping */
  m_rel_target_x = 0.;
  m_rel_target_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_total_weight = 0.;
  m_num_target_pairs = 0;
  m_warning_message.clear();
  m_warning_message.str("");
  m_abs_targets.clear();
  IncrementalConvexHull convex_hull;

  // loop through all possible pairs of neighbours in the formation whitelist
  for (map<string,PingContact>::iterator neighbour_1 = m_contact_whitelist.begin(); neighbour_1 != m_contact_whitelist.end(); ++neighbour_1) {  // loop through all neighbours in the whitelist
    if (neighbour_1->second.GetExpired()) {
//      m_warning_message << neighbour_1->first << " neighbour has not been 'seen' acoustically for more than (specified) " << BHV_AcousticPingPlanner::m_contact_timeout << " seconds.";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      continue;
    }

    for (map<string,PingContact>::iterator neighbour_2 = neighbour_1; neighbour_2 != m_contact_whitelist.end(); ++neighbour_2) {  // loop through the remaining neighbours
      if (neighbour_1->first == neighbour_2->first) {  // it's the same neighbour - cannot perform referencing (it's not a valid pair)
        continue;
      } else if (neighbour_2->second.GetExpired()) {
//        m_warning_message << neighbour_2->first << " neighbour has not been 'seen' acoustically for more than (specified) " << BHV_AcousticPingPlanner::m_contact_timeout << " seconds.";
//        postWMessage(m_warning_message.str());
//        m_warning_message.clear();
//        m_warning_message.str("");
        continue;
      }

      // Relative positions of this pair from the swarm plan (plus the midpoint between the pair)
      double x_1_rel_plan = m_swarm_plan[neighbour_1->first].x();
      double y_1_rel_plan = m_swarm_plan[neighbour_1->first].y();

      double x_2_rel_plan = m_swarm_plan[neighbour_2->first].x();
      double y_2_rel_plan = m_swarm_plan[neighbour_2->first].y();

      double x_mid_rel_plan = (x_1_rel_plan + x_2_rel_plan)/2.;
      double y_mid_rel_plan = (y_1_rel_plan + y_2_rel_plan)/2.;

      // Relative distance and angle to the midpoint of this pair from the swarm plan
      double dist_mid_rel_plan = hypot(x_mid_rel_plan, y_mid_rel_plan);
      double angle_mid_rel_plan = relAng(x_mid_rel_plan, y_mid_rel_plan, 0., 0.) - relAng(x_1_rel_plan, y_1_rel_plan, x_2_rel_plan, y_2_rel_plan);

      // Relative positions of this pair from acoustic pings (plus the midpoint between the pair)
      double x_1_rel_ping = neighbour_1->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
      double y_1_rel_ping = neighbour_1->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

      double x_2_rel_ping = neighbour_2->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
      double y_2_rel_ping = neighbour_2->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

      double x_mid_rel_ping = (x_1_rel_ping + x_2_rel_ping)/2.;
      double y_mid_rel_ping = (y_1_rel_ping + y_2_rel_ping)/2.;

      // Relative distance to the midpoint of this pair from acoustic pings
      double dist_mid_rel_ping = hypot(x_mid_rel_ping, y_mid_rel_ping);

      if (dist_mid_rel_ping > m_contact_rangeout) {
        m_warning_message << neighbour_1->first << " & " << neighbour_2->first << " neighbour pair midpoint is outside of rangeout distance " << m_contact_rangeout << ".";
        postWMessage(m_warning_message.str());
        m_warning_message.clear();
        m_warning_message.str("");
        continue;
      }

      // From the acoustic ping positions and midpoint of the pair, calculate our relative target position given the angle and distance of the pair's midpoint from the swarm plan
      double subtarget_x, subtarget_y;
      projectPoint(angle360(angle_mid_rel_plan + relAng(x_1_rel_ping, y_1_rel_ping, x_2_rel_ping, y_2_rel_ping)), dist_mid_rel_plan, x_mid_rel_ping, y_mid_rel_ping, subtarget_x, subtarget_y);

      // Calculate weighting according to the age of the pings of this pair
      double mean_pingtime = (neighbour_1->second.GetTimeLapse() + neighbour_2->second.GetTimeLapse())/2.;
      if (m_weight_pingtime) {
        m_pingtime_weight = 1-(mean_pingtime/BHV_AcousticPingPlanner::m_contact_timeout);
      } else {
        m_pingtime_weight = 1;
      }
      if (m_pingtime_weight < 0) {  //should not occur, since we break the loop if a contact has timed out
        m_pingtime_weight = 0.;
      }

      // Calculate weighting according to the midpoint distance of this pair
      if (m_weight_distance) {
        m_distance_weight = 1-(dist_mid_rel_ping/m_contact_rangeout);
      } else {
        m_distance_weight = 1;
      }

      // Add this calculated target position to the relative target, and weigh accordingly
      if (m_weight_averaging) {
        m_rel_target_x += m_pingtime_weight*m_distance_weight*subtarget_x;
        m_rel_target_y += m_pingtime_weight*m_distance_weight*subtarget_y;
        m_total_weight += m_pingtime_weight*m_distance_weight;
      } else {
        subtarget_x = m_pingtime_weight*m_distance_weight*subtarget_x;
        subtarget_y = m_pingtime_weight*m_distance_weight*subtarget_y;
        m_rel_target_x += subtarget_x;
        m_rel_target_y += subtarget_y;
        m_total_weight += 1;
      }
      m_num_target_pairs++;

      if (m_display_unaveraged_targets) {
        m_warning_message << m_us_name << "_pnr_" << m_num_target_pairs;
        m_abs_target.set_vertex(subtarget_x + BHV_DriftingTarget::m_nav_x, subtarget_y + BHV_DriftingTarget::m_nav_y);
        m_abs_target.set_color("vertex", "cyan");
        m_abs_target.set_label(m_warning_message.str());
        postMessage("VIEW_POINT", m_abs_target.get_spec());
        m_warning_message.clear();
        m_warning_message.str("");
      }

      if (m_display_unaveraged_hull) {
        convex_hull.addPoint(subtarget_x + BHV_DriftingTarget::m_nav_x, subtarget_y + BHV_DriftingTarget::m_nav_y);
      }
    }
  }

  if (m_display_unaveraged_targets && (m_num_target_pairs < m_num_target_pairs_prev)) {
    // remove previously drawn points that are no longer valid
    int diff = m_num_target_pairs_prev - m_num_target_pairs;
    for (unsigned int i = 1; i <= diff; i++) {
      m_warning_message << m_us_name << "_pnr_" << m_num_target_pairs + i;
      m_abs_target.set_vertex(0, 0);
      m_abs_target.set_color("vertex", "invisible");
      m_abs_target.set_label(m_warning_message.str());
      postMessage("VIEW_POINT", m_abs_target.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
    }
  }

  if (m_num_target_pairs >= 3 && m_display_unaveraged_hull) {
    // a polygon is only valid when there is more than 2 pair of contacts (three vertices needed)
    m_abs_targets = convex_hull.getConvexHullAsPolygon();
    m_abs_targets.set_label(m_us_name + "_pnr_tgts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "cyan");
    m_abs_targets.set_color("edge", "cyan");
    m_abs_targets.set_color("fill", "white");
    m_abs_targets.set_transparency(0.1);
    m_abs_targets.set_vertex_size(3);
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
    convex_hull.deleteAllPoints();
  } else if (m_display_unaveraged_hull && (m_num_target_pairs < m_num_target_pairs_prev)) {
    // remove previously drawn polygons - this occurs dynamically when there is only one pair of contacts left
    m_abs_targets.clear();
    m_abs_targets.add_vertex(0, 0);
    m_abs_targets.add_vertex(1, 0);
    m_abs_targets.add_vertex(1, 1);
    m_abs_targets.set_label(m_us_name + "_pnr_tgts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "invisible");
    m_abs_targets.set_color("edge", "invisible");
    m_abs_targets.set_color("fill", "invisible");
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
  }

  postMessage("PAIRWISE_NBR_REF_TOTAL_WEIGHT", m_total_weight);
  postMessage("PAIRWISE_NBR_REF_NUM_TARGET_PAIRS", m_num_target_pairs);
  m_rel_target_x = m_rel_target_x/m_total_weight;
  m_rel_target_y = m_rel_target_y/m_total_weight;
  m_num_target_pairs_prev = m_num_target_pairs;
  /* End relative target calculations here */

  if (m_num_target_pairs != 0) { // no neighbours exist - do not add a new relative target
    BHV_DriftingTarget::addRelativeTargetToFilter(m_rel_target_x, m_rel_target_y);
    // note: if no neighbours, what should we do? should we stay at the previous target? or should we return null and allow free drifting?
    // current: we allow free drifting by continuously setting the target to the current position
  } else {
    BHV_DriftingTarget::addRelativeTargetToFilter(0, 0);
  }

  return BHV_DriftingTarget::onRunState();
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display target points.

void BHV_PairwiseNeighbourReferencing::postViewablePoints()
{

}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_PairwiseNeighbourReferencing::postErasablePoints()
{
}
