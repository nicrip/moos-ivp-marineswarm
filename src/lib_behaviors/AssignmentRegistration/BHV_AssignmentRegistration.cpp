/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AssignmentRegistration.cpp                  */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <math.h>
#include <algorithm>
#include <limits>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_AssignmentRegistration.h"
#include "Geometry/IncrementalConvexHull.h"

using namespace std;
using namespace arma;

//---------------------------------------------------------------
// Constructor

BHV_AssignmentRegistration::BHV_AssignmentRegistration(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_AssignmentRegistration");

  // Initialize config variables
  m_contact_rangeout = 650.;
  m_display_rigid_registration_points = false;
  m_display_rigid_registration_hull = false;
  m_weight_targets = "";
  m_weight_averaging = false;
  m_ownship_weight = 1.0;

  // Initialize state variables
  m_rel_target_x = 0.;
  m_rel_target_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_weight_distance = true;
  m_weight_pingtime = true;
  m_total_weight = 0.;
  m_num_contacts = 0;
  m_num_contacts_prev = 0;
//  m_init_angle = 0.;
  m_warning_message.clear();
  m_warning_message.str("");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_AssignmentRegistration::setParam(string param, string val)
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
  } else if ((param == "display_rigid_registration_points")) {
    return(setBooleanOnString(m_display_rigid_registration_points, val));
  } else if ((param == "display_rigid_registration_hull")) {
    return(setBooleanOnString(m_display_rigid_registration_hull, val));
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
  } else if ((param == "ownship_weight") && non_neg_number) {
    m_ownship_weight = double_val;
    if (m_ownship_weight > 1) m_ownship_weight = 1.0;
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

void BHV_AssignmentRegistration::onSetParamComplete()
{
  BHV_DriftingTarget::onSetParamComplete();
  BHV_AcousticPingPlanner::onSetParamComplete();

  m_abs_target.set_color("label", "invisible");
  m_abs_target.set_color("vertex", "magenta");
  m_abs_target.set_vertex_size(10);

  m_abs_targets.set_color("label", "invisible");
  m_abs_targets.set_color("vertex", "magenta");
  m_abs_targets.set_color("edge", "magenta");
  m_abs_targets.set_color("fill", "white");
  m_abs_targets.set_transparency(0.1);
  m_abs_targets.set_vertex_size(3);
  m_abs_targets.set_label(m_us_name + "_scr_pnts");

  m_formation_points = mat(BHV_AcousticPingPlanner::m_original_swarm_plan.size(), 2);
  int i = 0;
  for(map<string,XYPoint>::iterator it = BHV_AcousticPingPlanner::m_original_swarm_plan.begin(); it != BHV_AcousticPingPlanner::m_original_swarm_plan.end(); ++it) {
    m_formation_points(i, 0) = it->second.x();
    m_formation_points(i, 1) = it->second.y();
    i++;
  }
  m_sorted_formation_points.clear();
  m_sorted_formation_dists.clear();
  sortFormationPoints(m_formation_points, m_sorted_formation_points, m_sorted_formation_dists);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_AssignmentRegistration::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_AssignmentRegistration::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_AssignmentRegistration::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_AssignmentRegistration::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_AssignmentRegistration::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_AssignmentRegistration::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_AssignmentRegistration::onRunState()
{
  if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  /* Begin relative target calculations here - this behavior uses a rigid transformation of the plan to the actual positions for formation keeping */
  m_rel_target_x = 0.;
  m_rel_target_y = 0.;
  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_total_weight = 0.;
  m_num_contacts = 0;
  m_warning_message.clear();
  m_warning_message.str("");

  int max_size = m_contact_whitelist.size();

  // neighbour ping positions
  m_nodes_points = mat(max_size+1, 2);
  m_nodes_points(0, 0) = 0;
  m_nodes_points(0, 1) = 0;

  // neighbour weights
  mat weights = mat(max_size+1, 1);
  weights(0, 0) = m_ownship_weight;

  // loop through all neighbours in the formation whitelist
  for (map<string,PingContact>::iterator neighbour = m_contact_whitelist.begin(); neighbour != m_contact_whitelist.end(); ++neighbour) {  // loop through all neighbours in the whitelist
    if (neighbour->second.GetExpired()) {
      continue;
    }

    // Relative positions of this neighbour from acoustic pings
    double x_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
    double y_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

    double dist_rel_ping = hypot(x_rel_ping, y_rel_ping);

    if (dist_rel_ping > m_contact_rangeout) {
      continue;
    }

    // add neighbour to matrices
    m_nodes_points(m_num_contacts+1, 0) = x_rel_ping;
    m_nodes_points(m_num_contacts+1, 1) = y_rel_ping;
    weights(m_num_contacts+1, 0) = 1.0;

    // Calculate weighting according to the age of the contact ping
    double pingtime = (neighbour->second.GetTimeLapse());
    if (m_weight_pingtime) {
      m_pingtime_weight = 1-(pingtime/BHV_AcousticPingPlanner::m_contact_timeout);
    } else {
      m_pingtime_weight = 1;
    }
    if (m_pingtime_weight < 0) {  //should not occur, since we break the loop if a contact has timed out
      m_pingtime_weight = 0.;
    }

    // Calculate weighting according to the distance of this contact
    if (m_weight_distance) {
      m_distance_weight = 1-(dist_rel_ping/m_contact_rangeout);
    } else {
      m_distance_weight = 1;
    }

    // Add this calculated target position to the relative target, and weigh accordingly
    if (m_weight_averaging) {
      m_total_weight += m_pingtime_weight*m_distance_weight;
    } else {
      m_total_weight += 1;
    }

    m_num_contacts++;
  }

  // remove additional space from neighbour matrices
  m_nodes_points.resize(m_num_contacts+1, 2);
  weights.resize(m_num_contacts+1, 1);

  vector<mat> scores;
  vector<umat> assignments;
  colvec min_costs;
  assignmentScores(m_nodes_points, 10, scores, assignments, min_costs);
  mat optimal_formation;
  optimalAssignment(scores, assignments, min_costs, optimal_formation);
  mat rotation;
  mat translation;
  mat solution;
  mat centroid_plan = (trans(weights)*optimal_formation)/accu(weights);
  mat centroid_ping = (trans(weights)*m_nodes_points)/accu(weights);
  rigidTransformation2D(optimal_formation, m_nodes_points, weights, centroid_plan, centroid_ping, rotation, translation, solution);
//    m_init_angle = atan2(rotation(1, 0), rotation(0, 0)); //init angle with rotation calculated

  if (m_display_rigid_registration_points) {
    for (unsigned int i = 0; i < solution.n_rows; i++) {
      m_warning_message << m_us_name << "_scr_" << i;
      m_abs_target.set_vertex(solution(i,0) + BHV_DriftingTarget::m_nav_x, solution(i,1) + BHV_DriftingTarget::m_nav_y);
      m_abs_target.set_color("vertex", "magenta");
      m_abs_target.set_label(m_warning_message.str());
      postMessage("VIEW_POINT", m_abs_target.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
    }
  }

  if (m_display_rigid_registration_points && (m_num_contacts < m_num_contacts_prev)) {
    // remove previously drawn points that are no longer valid
    int diff = m_num_contacts_prev - m_num_contacts;
    for (unsigned int i = 1; i <= diff; i++) {
      m_warning_message << m_us_name << "_scr_" << m_num_contacts + i;
      m_abs_target.set_vertex(0, 0);
      m_abs_target.set_color("vertex", "invisible");
      m_abs_target.set_label(m_warning_message.str());
      postMessage("VIEW_POINT", m_abs_target.get_spec());
      m_warning_message.clear();
      m_warning_message.str("");
    }
  }

  IncrementalConvexHull convex_hull;
  if (m_display_rigid_registration_hull) {
    for (unsigned int i = 0; i < solution.n_rows; i++) {
      srand(time(NULL) + i);
      double x_delta = ((double)rand()/(double)RAND_MAX)*0.1 - 0.05;
      double y_delta = ((double)rand()/(double)RAND_MAX)*0.1 - 0.05;
      // NOTE: SOMETHING IS WRONG WITH THE CHECKING OF POINTS WITHIN THE EXISTING CONVEX HULL (WHEN ANOTHER POINT IS AT THE EXACT INTERSECTION OF TWO EXISTING LINES) - THIS IS A HACKY WARKAROUND
      convex_hull.addPoint(solution(i,0) + BHV_DriftingTarget::m_nav_x + x_delta, solution(i,1) + BHV_DriftingTarget::m_nav_y + y_delta);
    }
  }

  if (m_num_contacts+1 >= 3 && m_display_rigid_registration_hull) {
    // a polygon is only valid when there is more than 2 pair of contacts (three vertices needed)
    m_abs_targets = convex_hull.getConvexHullAsPolygon();
    m_abs_targets.set_label(m_us_name + "_scr_pnts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "magenta");
    m_abs_targets.set_color("edge", "magenta");
    m_abs_targets.set_color("fill", "white");
    m_abs_targets.set_transparency(0.1);
    m_abs_targets.set_vertex_size(3);
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
    convex_hull.deleteAllPoints();
  } else if (m_display_rigid_registration_hull && (m_num_contacts < m_num_contacts_prev)) {
    // remove previously drawn polygons - this occurs dynamically when there is only one pair of contacts left
    m_abs_targets.clear();
    m_abs_targets.add_vertex(0, 0);
    m_abs_targets.add_vertex(1, 0);
    m_abs_targets.add_vertex(1, 1);
    m_abs_targets.set_label(m_us_name + "_scr_pnts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "invisible");
    m_abs_targets.set_color("edge", "invisible");
    m_abs_targets.set_color("fill", "invisible");
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
  }

  m_rel_target_x = solution(0, 0) * (m_total_weight/m_num_contacts);
  m_rel_target_y = solution(0, 1) * (m_total_weight/m_num_contacts);
  // NOTE: THIS WEIGHTING SOLUTION IS KIND OF A HACK! WE SHOULD PERFORM ACTUAL WEIGHTING IN THE POINT REGISTRATION INSTEAD! OR WE CAN LOOK AND SEE IF ONLY WE HAVE MOVED AND NOT OUR NEIGHBOURS TO TRIGGER NO CHANGE IN RELATIVE TARGET CALCULATION!

  postMessage("ASSIGN_REG_TOTAL_WEIGHT", m_total_weight);
  postMessage("ASSIGN_REG_NUM_CONTACTS", m_num_contacts);
  m_num_contacts_prev = m_num_contacts;
  /* End relative target calculations here */

  if (m_num_contacts != 0) { // no neighbours exist - do not add a new relative target
    BHV_DriftingTarget::addRelativeTargetToFilter(m_rel_target_x, m_rel_target_y);
    // note: if no neighbours, what should we do? should we stay at the previous target? or should we return null and allow free drifting?
    // current: we allow free drifting by continuously setting the target to the current position
  } else {
    BHV_DriftingTarget::addRelativeTargetToFilter(0, 0);
  }

  return BHV_DriftingTarget::onRunState();

/** EXAMPLE USAGE!!!**/
//  m_formation_points = mat(5, 2);
//  m_formation_points(0,0) = -100;
//  m_formation_points(0,1) = -100;
//  m_formation_points(1,0) = 100;
//  m_formation_points(1,1) = -100;
//  m_formation_points(2,0) = 100;
//  m_formation_points(2,1) = 100;
//  m_formation_points(3,0) = -100;
//  m_formation_points(3,1) = 100;
//  m_formation_points(4,0) = -100;
//  m_formation_points(4,1) = -300;
//  m_sorted_formation_points.clear();
//  m_sorted_formation_dists.clear();
//  sortFormationPoints(m_formation_points, m_sorted_formation_points, m_sorted_formation_dists);
//
//  m_nodes_points = mat(2,2);
//  m_nodes_points(0,0) = 50;
//  m_nodes_points(0,1) = 50;
//  m_nodes_points(1,0) = 50;
//  m_nodes_points(1,1) = -50;
//  vector<mat> scores;
//  vector<umat> assignments;
//  colvec min_costs;
//  assignmentScores(m_nodes_points, 30, scores, assignments, min_costs);
//  mat optimal_formation;
//  optimalAssignment(scores, assignments, min_costs, optimal_formation);
//  mat rotation;
//  mat translation;
//  mat solution;
//  mat weights = ones(m_nodes_points.n_rows, 1);
//  mat centroid_plan = (trans(weights)*optimal_formation)/accu(weights);
//  mat centroid_ping = (trans(weights)*m_nodes_points)/accu(weights);
//  rigidTransformation2D(optimal_formation, m_nodes_points, weights, centroid_plan, centroid_ping, rotation, translation, solution);
//
//  m_warning_message << endl << "OPTIMAL FORMATION TRANSFORMED" << endl;
//  m_warning_message << solution;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
}

//---------------------------------------------------------------
// Procedure: rigidTransformation2D()
//   Purpose: Perform calculations for rigid transformation.

void BHV_AssignmentRegistration::rigidTransformation2D(mat p, mat q, mat w, mat cent_p, mat cent_q, mat &R, mat &T, mat &sol)
{
  // Construct centered matrices
  mat x = p - repmat(cent_p, p.n_rows, 1);
  mat y = q - repmat(cent_q, q.n_rows, 1);

  // Construct covariance matrix
  mat H  = trans(x)*diagmat(w)*y;

  // Perform singular value decomposition
  mat U;
  vec s;
  mat V;
  svd(U, s, V, H);

  // Calculate rotation
  mat eyed;
  eyed.eye(H.n_rows, H.n_rows);
  eyed(H.n_rows-1, H.n_rows-1) = det(V*trans(U));
  R = V*eyed*trans(U);

  // Calculate translation
  T = trans(cent_q) - (R*trans(cent_p));

  // Perform transformation
  sol = R*trans(p) + repmat(T, 1, p.n_rows);
  sol = trans(sol);
}

//---------------------------------------------------------------
// Procedure: sortFormationPoints()
//   Purpose: For each point in the formation, sort all other points in order of Euclidean distance.

void BHV_AssignmentRegistration::sortFormationPoints(mat formation, std::vector<mat> &sorted_formation, std::vector<vec> &sorted_dists)
{
  vec dist = vec(formation.n_rows);
  for (unsigned int i = 0; i < formation.n_rows; i++) {
    double o_x = formation(i,0);
    double o_y = formation(i,1);
    for (unsigned int j = 0; j < formation.n_rows; j++) {
      double n_x = formation(j,0);
      double n_y = formation(j,1);
      dist(j) = hypot(n_x - o_x, n_y - o_y);
    }
    uvec sort_idxs = sort_index(dist);
    mat sort_form = formation.rows(sort_idxs);
    sorted_formation.push_back(sort_form);
    sorted_dists.push_back(sort(dist));
  }
}

//---------------------------------------------------------------
// Procedure: assignmentScores()
//   Purpose: For the nodes, look at each point in the formation and their n nearest neighbours, and calculate the Euclidean distance score matrix for a number of angles.

void BHV_AssignmentRegistration::assignmentScores(mat nodes, int angle_delta, std::vector<mat> &scores_form, std::vector<umat> &assignments, colvec &min_costs)
{
  // Number of rotations and delta change in angle
  int num_rot = (int)(360.0/angle_delta);
  mat rot = mat(2,2);
//  double sin_th = sin((angle_delta+m_init_angle)*M_PI/180.0);
//  double cos_th = cos((angle_delta+m_init_angle)*M_PI/180.0);
  double sin_th = sin(angle_delta*M_PI/180.0);
  double cos_th = cos(angle_delta*M_PI/180.0);
  rot(0,0) = cos_th;
  rot(0,1) = -sin_th;
  rot(1,0) = sin_th;
  rot(1,1) = cos_th;

  // Clear our vectors
  scores_form.clear();
  assignments.clear();
  min_costs.set_size(m_sorted_formation_points.size());

  // Get corresponding number of points from formation - selecting each point in the formation and a specific number of surrounding points, such that there are at least the same number of points, or any additional points still within the distance
  for (unsigned int j = 0; j < m_sorted_formation_points.size(); j++) {
    mat curr_pt_mat = m_sorted_formation_points[j];
    vec curr_pt_dists = m_sorted_formation_dists[j];
    int num_nearest_pts = nodes.n_rows;
    double max_dist = curr_pt_dists(num_nearest_pts - 1);
    double max_delta = 0.5;
    int idx = 0;
    for (vec::iterator it = curr_pt_dists.begin()+num_nearest_pts; it != curr_pt_dists.end(); ++it) {
      if (abs(max_dist - *it) < max_delta) {
        idx++;
      } else {
        break;
      }
    }
    num_nearest_pts += idx;
    curr_pt_mat = curr_pt_mat.rows(0, num_nearest_pts-1);
    mat centroid_curr_pt_mat = (ones(1, curr_pt_mat.n_rows)*curr_pt_mat)/curr_pt_mat.n_rows;
    curr_pt_mat = curr_pt_mat - repmat(centroid_curr_pt_mat, curr_pt_mat.n_rows, 1);

    mat centroid_nodes = (ones(1, nodes.n_rows)*nodes)/nodes.n_rows;
    mat nodes_offset = nodes - repmat(centroid_nodes, nodes.n_rows, 1);

    int rows = curr_pt_mat.n_rows;
    int cols = nodes_offset.n_rows;
    mat costs = mat(rows, cols);

    if (rows > cols) {
      costs.insert_cols(nodes_offset.n_rows, rows - cols);
    }

    double min_cost = std::numeric_limits<double>::max();
    double curr_cost;
    int min_angle = 0;
    umat min_assignment(rows, cols);
    umat curr_assignment(rows, cols);
    for (unsigned int i = 0; i < num_rot; i++) {
      nodes_offset = trans(rot*trans(nodes_offset));
      for (unsigned int k = 0; k < nodes_offset.n_rows; k++) {
        for (unsigned int l = 0; l < curr_pt_mat.n_rows; l++) {
          costs(l,k) = (hypot(nodes_offset(k,0) - curr_pt_mat(l,0), nodes_offset(k,1) - curr_pt_mat(l,1)));
        }
      }
      curr_assignment = hungarian(costs);
      curr_cost = accu(curr_assignment%costs);
      if (curr_cost < min_cost) {
        min_cost = curr_cost;
        min_assignment = curr_assignment;
        min_angle = (i+1)*angle_delta;
      }
    }
    scores_form.push_back(curr_pt_mat);
    assignments.push_back(min_assignment);
    min_costs(j) = min_cost;
  }
}

//---------------------------------------------------------------
// Procedure: optimalAssignment()
//   Purpose: Select the formation and assignments with the lowest cost, and set the assigned matrix as the desired formation.

void BHV_AssignmentRegistration::optimalAssignment(std::vector<mat> &scores_form, std::vector<umat> &assignments, colvec &min_costs, mat &optimal_formation)
{
  uvec sort_idxs = sort_index(min_costs);
  int best_idx = sort_idxs(0);
  mat best_formation = scores_form[best_idx];
  umat optimal_assignment = assignments[best_idx];

  optimal_formation = mat(m_nodes_points.n_rows, m_nodes_points.n_cols);
  umat indexes = linspace<umat>(0, optimal_assignment.n_rows-1, optimal_assignment.n_rows);

  indexes = trans(optimal_assignment)*indexes;
  for (unsigned int i = 0; i < optimal_formation.n_rows; i++) {
    optimal_formation(i, 0) = best_formation(indexes(i), 0);
    optimal_formation(i, 1) = best_formation(indexes(i), 1);
  }
}

/**MUNKRES ASSIGNMENT ALGORITHM START**/ //Lars Simon Zehnder - http://gallery.rcpp.org/articles/minimal-assignment/
void BHV_AssignmentRegistration::step_one(unsigned int &step, arma::mat &cost,
        const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        cost.row(r) -= arma::min(cost.row(r));
    }
    step = 2;
}

void BHV_AssignmentRegistration::step_two (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (cost.at(r, c) == 0.0 && rcov.at(r) == 0 && ccov.at(c) == 0) {
                indM.at(r, c)  = 1;
                rcov.at(r)     = 1;
                ccov.at(c)     = 1;
                break;                                              // Only take the first
                                                                    // zero in a row and column
            }
        }
    }
    /* for later reuse */
    rcov.fill(0);
    ccov.fill(0);
    step = 3;
}

void BHV_AssignmentRegistration::step_three(unsigned int &step, const arma::umat &indM,
        arma::ivec &ccov, const unsigned int &N)
{
    unsigned int colcount = 0;
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (indM.at(r, c) == 1) {
                ccov.at(c) = 1;
            }
        }
    }
    for (unsigned int c = 0; c < N; ++c) {
        if (ccov.at(c) == 1) {
            ++colcount;
        }
    }
    if (colcount == N) {
        step = 7;
    } else {
        step = 4;
    }
}

void BHV_AssignmentRegistration::find_noncovered_zero(int &row, int &col,
        const arma::mat &cost, const arma::ivec &rcov,
        const arma::ivec &ccov, const unsigned int &N)
{
    unsigned int r = 0;
    unsigned int c;
    bool done = false;
    row = -1;
    col = -1;
    while (!done) {
        c = 0;
        while (true) {
            if (cost.at(r, c) == 0.0 && rcov.at(r) == 0 && ccov.at(c) == 0) {
                row = r;
                col = c;
                done = true;
            }
            ++c;
            if (c == N || done) {
                break;
            }
        }
        ++r;
        if (r == N) {
            done = true;
        }
    }
}

bool BHV_AssignmentRegistration::star_in_row(int &row, const arma::umat &indM,
        const unsigned int &N)
{
    bool tmp = false;
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 1) {
            tmp = true;
            break;
        }
    }
    return tmp;
}

void BHV_AssignmentRegistration::find_star_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N)
{
    col = -1;
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 1) {
            col = c;
        }
    }
}

void BHV_AssignmentRegistration::step_four (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov, arma::ivec &ccov,
        int &rpath_0, int &cpath_0, const unsigned int &N)
{
    int row = -1;
    int col = -1;
    bool done = false;
    while(!done) {
        find_noncovered_zero(row, col, cost, rcov,
                ccov, N);

        if (row == -1) {
            done = true;
            step = 6;
        } else {
            /* uncovered zero */
            indM(row, col) = 2;
            if (star_in_row(row, indM, N)) {
                find_star_in_row(row, col, indM, N);
                /* Cover the row with the starred zero
                 * and uncover the column with the starred
                 * zero.
                 */
                rcov.at(row) = 1;
                ccov.at(col) = 0;
            } else {
                /* No starred zero in row with
                 * uncovered zero
                 */
                done = true;
                step = 5;
                rpath_0 = row;
                cpath_0 = col;
            }
        }
    }
}

void BHV_AssignmentRegistration::find_star_in_col (const int &col, int &row,
        const arma::umat &indM, const unsigned int &N)
{
    row = -1;
    for (unsigned int r = 0; r < N; ++r) {
        if (indM.at(r, col) == 1) {
            row = r;
        }
    }
}

void BHV_AssignmentRegistration::find_prime_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N)
{
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 2) {
            col = c;
        }
    }
}

void BHV_AssignmentRegistration::augment_path (const int &path_count, arma::umat &indM,
        const arma::imat &path)
{
    for (unsigned int p = 0; p < path_count; ++p) {
        if (indM.at(path(p, 0), path(p, 1)) == 1) {
            indM.at(path(p, 0), path(p, 1)) = 0;
        } else {
            indM.at(path(p, 0), path(p, 1)) = 1;
        }
    }
}

void BHV_AssignmentRegistration::clear_covers (arma::ivec &rcov, arma::ivec &ccov)
{
    rcov.fill(0);
    ccov.fill(0);
}

void BHV_AssignmentRegistration::erase_primes(arma::umat &indM, const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (indM.at(r, c) == 2) {
                indM.at(r, c) = 0;
            }
        }
    }
}

void BHV_AssignmentRegistration::step_five (unsigned int &step,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, arma::imat &path,
        int &rpath_0, int &cpath_0,
        const unsigned int &N)
{
    bool done = false;
    int row = -1;
    int col = -1;
    unsigned int path_count = 1;
    path.at(path_count - 1, 0) = rpath_0;
    path.at(path_count - 1, 1) = cpath_0;
    while (!done) {
        find_star_in_col(path.at(path_count - 1, 1), row,
                indM, N);
        if (row > -1) {
            /* Starred zero in row 'row' */
            ++path_count;
            path.at(path_count - 1, 0) = row;
            path.at(path_count - 1, 1) = path.at(path_count - 2, 1);
        } else {
            done = true;
        }
        if (!done) {
            /* If there is a starred zero find a primed
             * zero in this row; write index to 'col' */
            find_prime_in_row(path.at(path_count - 1, 0), col,
                    indM, N);
            ++path_count;
            path.at(path_count - 1, 0) = path.at(path_count - 2, 0);
            path.at(path_count - 1, 1) = col;
        }
    }
    augment_path(path_count, indM, path);
    clear_covers(rcov, ccov);
    erase_primes(indM, N);
    step = 3;
}

void BHV_AssignmentRegistration::find_smallest (double &minval, const arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (rcov.at(r) == 0 && ccov.at(c) == 0) {
                if (minval > cost.at(r, c)) {
                    minval = cost.at(r, c);
                }
            }
        }
    }
}

void BHV_AssignmentRegistration::step_six (unsigned int &step, arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N)
{
    double minval = std::numeric_limits<double>::max();;
    find_smallest(minval, cost, rcov, ccov, N);
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (rcov.at(r) == 1) {
                cost.at(r, c) += minval;
            }
            if (ccov.at(c) == 0) {
                cost.at(r, c) -= minval;
            }
        }
    }
    step = 4;
}

arma::umat BHV_AssignmentRegistration::hungarian(const arma::mat &input_cost)
{
    const unsigned int N = input_cost.n_rows;
    unsigned int step = 1;
    int cpath_0 = 0;
    int rpath_0 = 0;
    arma::mat cost(input_cost);
    arma::umat indM(N, N);
    arma::ivec rcov(N);
    arma::ivec ccov(N);
    arma::imat path(2 * N, 2);

    indM = arma::zeros<arma::umat>(N, N);
    bool done = false;
    while (!done) {
        switch (step) {
            case 1:
                step_one(step, cost, N);
                break;
            case 2:
                step_two(step, cost, indM, rcov, ccov, N);
                break;
            case 3:
                step_three(step, indM, ccov, N);
                break;
            case 4:
                step_four(step, cost, indM, rcov, ccov,
                        rpath_0, cpath_0, N);
                break;
            case 5:
                step_five(step, indM, rcov, ccov,
                        path, rpath_0, cpath_0, N);
                break;
            case 6:
                step_six(step, cost, rcov, ccov, N);
                break;
            case 7:
                done = true;
                break;
        }
    }
    return indM;
}
/**MUNKRES ASSIGNMENT ALGORITHM END**/
