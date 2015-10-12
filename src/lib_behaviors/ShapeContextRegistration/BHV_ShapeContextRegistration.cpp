/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ShapeContextRegistration.cpp                */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <math.h>
#include <algorithm>
#include <limits>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_ShapeContextRegistration.h"
#include "Geometry/IncrementalConvexHull.h"

using namespace std;
using namespace arma;

//---------------------------------------------------------------
// Constructor

BHV_ShapeContextRegistration::BHV_ShapeContextRegistration(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_ShapeContextRegistration");

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

bool BHV_ShapeContextRegistration::setParam(string param, string val)
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

void BHV_ShapeContextRegistration::onSetParamComplete()
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
  shapeContexts(m_formation_points, 20.0, 1e8, m_formation_shape_contexts);

  m_warning_message << "formation context" << endl;
  for (int i=0; i<m_formation_shape_contexts.size(); i++) {
    for (int j=0; j<m_formation_shape_contexts[i].size(); j++) {
      m_warning_message << "[";
      for (int k=0; k<m_formation_shape_contexts[i][j].size(); k++) {
        m_warning_message << m_formation_shape_contexts[i][j][k] << " ";
      }
      m_warning_message << "] ";
    }
    m_warning_message << endl;
  }
  postWMessage(m_warning_message.str());
  m_warning_message.str("");
  m_warning_message.clear();

}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_ShapeContextRegistration::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_ShapeContextRegistration::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_ShapeContextRegistration::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_ShapeContextRegistration::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_ShapeContextRegistration::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_ShapeContextRegistration::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_ShapeContextRegistration::onRunState()
{
    if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  // ALGORITHM INITAL TEST
//  m_formation_points = mat(9, 2);
//  m_formation_points(0,0) = 0;
//  m_formation_points(0,1) = 0;
//  m_formation_points(1,0) = 0;
//  m_formation_points(1,1) = 100;
//  m_formation_points(2,0) = 0;
//  m_formation_points(2,1) = 200;
//  m_formation_points(3,0) = 0;
//  m_formation_points(3,1) = 300;
//  m_formation_points(4,0) = 50;
//  m_formation_points(4,1) = 200;
//  m_formation_points(5,0) = 100;
//  m_formation_points(5,1) = 100;
//  m_formation_points(6,0) = 150;
//  m_formation_points(6,1) = 200;
//  m_formation_points(7,0) = 200;
//  m_formation_points(7,1) = 300;
//  m_formation_points(8,0) = 200;
//  m_formation_points(8,1) = 200;
//  m_formation_points(9,0) = 200;
//  m_formation_points(9,1) = 100;
//  m_formation_points(10,0) = 200;
//  m_formation_points(10,1) = 0;
//  shapeContexts(m_formation_points, 20.0, 1e8, m_formation_shape_contexts);
//
//  m_nodes_points = mat(9, 2);
//  m_nodes_points(0,0) = 243.6070;
//  m_nodes_points(0,1) = 169.0497;
//  m_nodes_points(1,0) = 138.2520;
//  m_nodes_points(1,1) = -21.0994;
//  m_nodes_points(2,0) = 361.3677;
//  m_nodes_points(2,1) = 3.8511;
//  m_nodes_points(3,0) = 117.6992;
//  m_nodes_points(3,1) = -150.5232;
//  m_nodes_points(4,0) = 291.7538;
//  m_nodes_points(4,1) = -53.0126;
//  m_nodes_points(5,0) = 195.0101;
//  m_nodes_points(5,1) = 73.3815;
//  m_nodes_points(6,0) = 5.4129;
//  m_nodes_points(6,1) = 4.7788;
//  m_nodes_points(7,0) = 236.9770;
//  m_nodes_points(7,1) = -12.3963;
//  m_nodes_points(8,0) = 165.3048;
//  m_nodes_points(8,1) = 98.5641;
////  m_nodes_points(9,0) = 198.9448;
////  m_nodes_points(9,1) = -118.4620;
////  m_nodes_points(10,0) = 90.8447;
////  m_nodes_points(10,1) = 64.9219;
//  shapeContexts(m_nodes_points, 20.0, 1e8, m_nodes_shape_contexts);
//
//  mat new_formation_points;
//  optimalCorrespondences(new_formation_points);
//
//  // Rotation, Translation and Solution
//  mat rotation;
//  mat translation;
//  mat solution;
//  mat weights = ones(9, 1);
//  mat centroid_plan = (trans(weights)*new_formation_points)/accu(weights);
//  mat centroid_ping = (trans(weights)*m_nodes_points)/accu(weights);
//
//  // Calculate the rigid transformation
//  rigidTransformation2D(new_formation_points, m_nodes_points, weights, centroid_plan, centroid_ping, rotation, translation, solution);
//
//  m_warning_message << endl << solution << endl;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");

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
//      m_warning_message << neighbour->first << " neighbour has not been 'seen' acoustically for more than (specified) " << BHV_AcousticPingPlanner::m_contact_timeout << " seconds.";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      continue;
    }

    // Relative positions of this neighbour from acoustic pings
    double x_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
    double y_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

    double dist_rel_ping = hypot(x_rel_ping, y_rel_ping);

    if (dist_rel_ping > m_contact_rangeout) {
//      m_warning_message << neighbour->first << " neighbour is outside of rangeout distance " << m_contact_rangeout << ".";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
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

  cout << m_formation_points << endl;
//  cout << m_nodes_points << endl;

  // Calculate the modified shape contexts
  shapeContexts(m_formation_points, 20.0, 1e8, m_formation_shape_contexts);
  shapeContexts(m_nodes_points, 20.0, 1e8, m_nodes_shape_contexts);

  m_warning_message << "nodes context" << endl;
  for (int i=0; i<m_nodes_shape_contexts.size(); i++) {
    for (int j=0; j<m_nodes_shape_contexts[i].size(); j++) {
      m_warning_message << "[";
      for (int k=0; k<m_nodes_shape_contexts[i][j].size(); k++) {
        m_warning_message << m_nodes_shape_contexts[i][j][k] << " ";
      }
      m_warning_message << "] ";
    }
    m_warning_message << endl;
  }
  postWMessage(m_warning_message.str());
  m_warning_message.str("");
  m_warning_message.clear();

  // Determine the optimal correspondences
  mat new_formation_points;
  optimalCorrespondences(new_formation_points);

  // Calculate the centroids
  mat centroid_formation = (trans(weights)*new_formation_points)/accu(weights);
  mat centroid_nodes = (trans(weights)*m_nodes_points)/accu(weights);

  // Rotation, Translation and Solution
  mat rotation;
  mat translation;
  mat solution;

  // Calculate the rigid transformation
  rigidTransformation2D(new_formation_points, m_nodes_points, weights, centroid_formation, centroid_nodes, rotation, translation, solution);

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

  postMessage("RIGID_NBR_REG_TOTAL_WEIGHT", m_total_weight);
  postMessage("RIGID_NBR_REG_NUM_CONTACTS", m_num_contacts);
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
}

//---------------------------------------------------------------
// Procedure: rigidTransformation2D()
//   Purpose: Perform calculations for rigid transformation.

void BHV_ShapeContextRegistration::rigidTransformation2D(mat p, mat q, mat w, mat cent_p, mat cent_q, mat &R, mat &T, mat &sol)
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

  // Perform transformation on original complete formation -> should we do this?
//  m_formation_points = R*trans(m_formation_points) + repmat(T, 1, m_formation_points.n_rows);
//  m_formation_points = trans(m_formation_points);
}

//---------------------------------------------------------------
// Procedure: shapeContext()
//   Purpose: Calculate modified shape contexts for a point set (modified shape context for each point in the set).

void BHV_ShapeContextRegistration::shapeContexts(mat f, double ang_delta, double max_dist, std::vector< std::vector< std::vector<double> > > &shape_contexts)
{
  shape_contexts.clear();
  int r = f.n_rows;

  for (unsigned int i = 0; i < r; i++) {
    vector< vector<double> > descriptor;
    descriptor.resize((int)(360.0/ang_delta));
    double os_x = f(i, 0);
    double os_y = f(i, 1);
    for (unsigned int j = 0; j < r; j++) {
      if (i == j) continue;
      double nbr_x = f(j, 0);
      double nbr_y = f(j, 1);
      double angle = atan2(nbr_y - os_y, nbr_x - os_x)*180/M_PI;
      double dist = hypot(nbr_y - os_y, nbr_x - os_x);
      unsigned int desc_index = round(((angle) + 180.0)/ang_delta);
      if (dist < max_dist) {
        if (desc_index == descriptor.size()) desc_index = 0;
        descriptor[desc_index].push_back(dist);
      }
    }
    shape_contexts.push_back(descriptor);
  }
}

//---------------------------------------------------------------
// Procedure: shapeContextScore()
//   Purpose: Calculate the score between two shape contexts, for all shifts in angular bins.

void BHV_ShapeContextRegistration::shapeContextScores(std::vector< std::vector<double> > &shape_context_node, std::vector< std::vector<double> > &shape_context_formation, Cube<int> &scores)
{
  // we might be able to make this more efficient by purposefully not performing a circular shift
  int len = shape_context_node.size();
  scores.zeros(1,1,len);

  for (unsigned int i = 0; i < len; i++) {
    double score = 0.0;
    for (unsigned int j = 0; j < len; j++) {
      vector<double> node_sc_dists = shape_context_node[j];
      vector<double> form_sc_dists = shape_context_formation[j];
      for (unsigned int k = 0; k < node_sc_dists.size(); k++) {
        double min_diff = node_sc_dists[k];
        for (unsigned int l = 0; l < form_sc_dists.size(); l++) {
          double diff = abs(node_sc_dists[k] - form_sc_dists[l]);
          if (diff < min_diff) min_diff = diff;
        }
        score += min_diff;
      }
    }
    scores(0,0,i) = round(score);
    rotate(shape_context_node.begin(), shape_context_node.begin()+1, shape_context_node.end());
  }
}

//---------------------------------------------------------------
// Procedure: optimalCorrespondences()
//   Purpose: Determine the optimal correspondences by scoring point-to-point and using the Hungarian algorithm for assignment between the nodes and formation shape contexts.

void BHV_ShapeContextRegistration::optimalCorrespondences(mat &new_formation_points)
{
  int rows = m_formation_shape_contexts.size();
  int cols = m_nodes_shape_contexts.size();
  int slices = m_nodes_shape_contexts[0].size();
  Cube<int> all_scores_all_shifts_all_points = Cube<int>(cols, rows, slices);
  for (unsigned int i = 0; i < m_nodes_shape_contexts.size(); i++) {
    for (unsigned int j = 0; j < m_formation_shape_contexts.size(); j++) {
      Cube<int> scores_all_shifts;
      shapeContextScores(m_nodes_shape_contexts[i], m_formation_shape_contexts[j], scores_all_shifts);
      all_scores_all_shifts_all_points.tube(i,j) = scores_all_shifts;
    }
  }
  all_scores_all_shifts_all_points.reshape(rows*cols, 1, slices);

  int min_cost = std::numeric_limits<int>::max();
  vector< pair<int, int> > min_assignment(rows);
  for (unsigned int i = 0; i < slices; i++) {
    int** m = matToArrayArray(all_scores_all_shifts_all_points.slice(i), cols, rows);
    int matrix_size = hungarian_init(&m_hungarian, m , cols, rows, HUNGARIAN_MODE_MINIMIZE_COST);
    hungarian_solve(&m_hungarian);
    if (m_hungarian.opt_cost < min_cost) {
      min_cost = m_hungarian.opt_cost;
      min_assignment = m_hungarian.opt_assignment;
    }
    free(m);
  }

//  m_warning_message << endl << min_cost << endl;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
//  for(int j=0; j<rows; j++) {
//    m_warning_message << min_assignment[j].first << ", " << min_assignment[j].second << endl;
//  }
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");

  new_formation_points = mat(m_nodes_points.n_rows, m_nodes_points.n_cols);
  for (unsigned int i = 0; i < cols; i++) {
    new_formation_points(i,0) = m_formation_points(min_assignment[i].second,0);
    new_formation_points(i,1) = m_formation_points(min_assignment[i].second,1);
  }

//  m_warning_message << endl << new_formation_points << endl;
//  postWMessage(m_warning_message.str());
//  m_warning_message.clear();
//  m_warning_message.str("");
}

//---------------------------------------------------------------
// Procedure: matToVec()
//   Purpose: Convert an armadillo matrix to a vector of vectors.

int** BHV_ShapeContextRegistration::matToArrayArray(Mat<int> &m, int rows, int cols)
{
  int *aa = m.memptr();

  int i,j;
  int** r;
  r = (int**)calloc(rows,sizeof(int*));
  for(i=0;i<rows;i++)
  {
    r[i] = (int*)calloc(cols,sizeof(int));
    for(j=0;j<cols;j++)
      r[i][j] = aa[i*cols+j];
  }
  return r;
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display target points.

void BHV_ShapeContextRegistration::postViewablePoints()
{

}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_ShapeContextRegistration::postErasablePoints()
{
}
