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
//  m_warning_message << "original" << endl << m_formation_points << endl;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
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

  m_formation_points = mat(5, 2);
  m_formation_points(0,0) = -100;
  m_formation_points(0,1) = -100;
  m_formation_points(1,0) = 100;
  m_formation_points(1,1) = -100;
  m_formation_points(2,0) = 100;
  m_formation_points(2,1) = 100;
  m_formation_points(3,0) = -100;
  m_formation_points(3,1) = 100;
  m_formation_points(4,0) = -100;
  m_formation_points(4,1) = -300;
  m_sorted_formation_points.clear();
  m_sorted_formation_dists.clear();
  sortFormationPoints(m_formation_points, m_sorted_formation_points, m_sorted_formation_dists);

  m_nodes_points = mat(2,2);
  m_nodes_points(0,0) = 50;
  m_nodes_points(0,1) = 50;
  m_nodes_points(1,0) = 50;
  m_nodes_points(1,1) = -50;
  vector<mat> scores;
  vector< std::vector< std::pair<int, int> > > assignments;
  Col<int> min_costs;
  assignmentScores(m_nodes_points, 30, scores, assignments, min_costs);
  mat optimal_formation;
  optimalAssignment(scores, assignments, min_costs, optimal_formation);
  mat rotation;
  mat translation;
  mat solution;
  mat weights = ones(m_nodes_points.n_rows, 1);
  mat centroid_plan = (trans(weights)*optimal_formation)/accu(weights);
  mat centroid_ping = (trans(weights)*m_nodes_points)/accu(weights);
  rigidTransformation2D(optimal_formation, m_nodes_points, weights, centroid_plan, centroid_ping, rotation, translation, solution);

  m_warning_message << endl << "OPTIMAL FORMATION TRANSFORMED" << endl;
  m_warning_message << solution;
  postWMessage(m_warning_message.str());
  m_warning_message.clear();
  m_warning_message.str("");

//  if (m_num_contacts != 0) { // no neighbours exist - do not add a new relative target
//    BHV_DriftingTarget::addRelativeTargetToFilter(m_rel_target_x, m_rel_target_y);
//    // note: if no neighbours, what should we do? should we stay at the previous target? or should we return null and allow free drifting?
//    // current: we allow free drifting by continuously setting the target to the current position
//  } else {
//    BHV_DriftingTarget::addRelativeTargetToFilter(0, 0);
//  }
//
//  return BHV_DriftingTarget::onRunState();

  return NULL;
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

void BHV_AssignmentRegistration::assignmentScores(mat nodes, int angle_delta, std::vector<mat> &scores_form, std::vector< std::vector< std::pair<int, int> > > &assignments, Col<int> &min_costs)
{
  // Number of rotations and delta change in angle
  int num_rot = (int)(360.0/angle_delta);
  mat rot = mat(2,2);
  double sin_th = sin(angle_delta*M_PI/180.0);
  double cos_th = cos(angle_delta*M_PI/180.0);
  rot(0,0) = cos_th;
  rot(0,1) = -sin_th;
  rot(1,0) = sin_th;
  rot(1,1) = cos_th;

  // Clear our vectors
  scores_form.clear();
  assignments.clear();
  min_costs = Col<int>(m_sorted_formation_points.size());

//  m_warning_message << "FORMATION SIZE" << endl;
//  m_warning_message << m_sorted_formation_points.size() << endl;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");

  // Get corresponding number of points from formation - selecting each point in the formation and a specific number of surrounding points, such that there are at least the same number of points, or any additional points still within the distance
  for (unsigned int j = 0; j < m_sorted_formation_points.size(); j++) { //m_sorted_formation_points.size()
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

    /****/
//    m_warning_message << "FORMATION " << j << endl;
//    m_warning_message << curr_pt_mat << endl;
//    postWMessage(m_warning_message.str());
//    m_warning_message.clear();
//    m_warning_message.str("");
    /****/

    int rows = curr_pt_mat.n_rows;
    int cols = nodes.n_rows;
    Mat<int> costs = Mat<int>(rows, cols);
    int min_cost = std::numeric_limits<int>::max();
    int min_angle = 0;
    vector< pair<int, int> > min_assignment(rows);
    for (unsigned int i = 0; i < num_rot; i++) { //num_rot
      nodes = trans(rot*trans(nodes));
      mat centroid_nodes = (ones(1, nodes.n_rows)*nodes)/nodes.n_rows;
      mat nodes_offset = nodes - repmat(centroid_nodes, nodes.n_rows, 1);
      Col<int> costs_vec = Col<int>(rows*cols);
      for (unsigned int k = 0; k < nodes_offset.n_rows; k++) {
        for (unsigned int l = 0; l < curr_pt_mat.n_rows; l++) {
          costs(l,k) = (int)round(hypot(nodes_offset(k,0) - curr_pt_mat(l,0), nodes_offset(k,1) - curr_pt_mat(l,1)));
        }
      }

      /****/
//      m_warning_message << (i+1)*angle_delta << endl << trans(costs);
      /****/

      costs_vec = vectorise(costs);
      int** m = matToArrayArray(costs_vec, cols, rows);
      int matrix_size = hungarian_init(&m_hungarian, m , cols, rows, HUNGARIAN_MODE_MINIMIZE_COST);
      hungarian_solve(&m_hungarian);
      if (m_hungarian.opt_cost < min_cost) {
        min_cost = m_hungarian.opt_cost;
        min_assignment = m_hungarian.opt_assignment;
        min_angle = (i+1)*angle_delta;
      }
      free(m);

      /****/
//      m_warning_message << "COST " << m_hungarian.opt_cost << endl;
//      m_warning_message << "[";
//      for(int z=0; z<rows; z++) {
//        m_warning_message << "[" << m_hungarian.opt_assignment[z].first << ", " << m_hungarian.opt_assignment[z].second << "]";
//      }
//      m_warning_message << "]";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      /****/

    }

    /****/
//    m_warning_message << "MIN COST " << min_cost << ", " << min_angle << endl;
//    m_warning_message << "[";
//    for(int z=0; z<rows; z++) {
//      m_warning_message << "[" << min_assignment[z].first << ", " << min_assignment[z].second << "]";
//    }
//    m_warning_message << "]";
//    postWMessage(m_warning_message.str());
//    m_warning_message.clear();
//    m_warning_message.str("");
    /****/

    scores_form.push_back(curr_pt_mat);
    assignments.push_back(min_assignment);
    min_costs(j) = min_cost;
  }

  /****/
//  m_warning_message << "COSTS " << endl;
//  m_warning_message << min_costs;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
  /****/
}

//---------------------------------------------------------------
// Procedure: matToVec()
//   Purpose: Convert an armadillo matrix to a vector of vectors.

int** BHV_AssignmentRegistration::matToArrayArray(Col<int> &m, int rows, int cols)
{
  int *aa = m.memptr();

//  m_warning_message << rows << " x " << cols << endl;

  int i,j;
  int** r;
  r = (int**)calloc(rows,sizeof(int*));
  for(i=0;i<rows;i++)
  {
//    m_warning_message << " row " << i;
    r[i] = (int*)calloc(cols,sizeof(int));
    for(j=0;j<cols;j++) {
//      m_warning_message << " col " << j;
      r[i][j] = aa[i*cols+j];
//      m_warning_message << " cost " << aa[i*cols+j] << " | ";
    }
  }
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
  return r;
}

//---------------------------------------------------------------
// Procedure: optimalAssignment()
//   Purpose: Select the formation and assignments with the lowest cost, and set the assigned matrix as the desired formation.

void BHV_AssignmentRegistration::optimalAssignment(std::vector<mat> &scores_form, std::vector< std::vector< std::pair<int, int> > > &assignments, Col<int> &min_costs, mat &optimal_formation)
{
  uvec sort_idxs = sort_index(min_costs);
  int best_idx = sort_idxs(0);
  mat best_formation = scores_form[best_idx];
  vector< pair<int, int> > optimal_assignment = assignments[best_idx];

  optimal_formation = mat(m_nodes_points.n_rows, m_nodes_points.n_cols);
  for (unsigned int i = 0; i < optimal_formation.n_rows; i++) {
    optimal_formation(i, 0) = best_formation(optimal_assignment[i].second, 0);
    optimal_formation(i, 1) = best_formation(optimal_assignment[i].second, 1);
  }

  m_warning_message << endl << "BEST COST" << endl;
  m_warning_message << min_costs(best_idx) << endl;
  m_warning_message << "BEST COST FORMATION" << endl;
  m_warning_message << best_formation;
  m_warning_message << "OPTIMAL FORMATION" << endl;
  m_warning_message << optimal_formation;
  m_warning_message << "BEST ASSIGNMENT " << endl;
  m_warning_message << "[";
  for(int z=0; z<optimal_assignment.size(); z++) {
    m_warning_message << "[" << optimal_assignment[z].first << ", " << optimal_assignment[z].second << "]";
  }
  m_warning_message << "]";
  postWMessage(m_warning_message.str());
  m_warning_message.clear();
  m_warning_message.str("");
}
