/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_RigidNeighbourRegistrationOpt.cpp           */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_RigidNeighbourRegistrationOpt.h"
#include "Geometry/IncrementalConvexHull.h"

using namespace std;
using namespace arma;

//---------------------------------------------------------------
// Constructor

BHV_RigidNeighbourRegistrationOpt::BHV_RigidNeighbourRegistrationOpt(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_RigidNeighbourRegistrationOpt");

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
  m_rel_target_x_prev = 0.;
  m_rel_target_y_prev = 0.;
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

bool BHV_RigidNeighbourRegistrationOpt::setParam(string param, string val)
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

void BHV_RigidNeighbourRegistrationOpt::onSetParamComplete()
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
  m_abs_targets.set_label(m_us_name + "_rnr_pnts");
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_RigidNeighbourRegistrationOpt::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_RigidNeighbourRegistrationOpt::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_RigidNeighbourRegistrationOpt::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_RigidNeighbourRegistrationOpt::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_RigidNeighbourRegistrationOpt::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_RigidNeighbourRegistrationOpt::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_RigidNeighbourRegistrationOpt::onRunState()
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

  // centroid of ping positions
  double x_centroid_ping = 0.0;
  double y_centroid_ping = 0.0;

  // centroid of plan positions
  double x_centroid_plan = 0.0;
  double y_centroid_plan = 0.0;

  int max_size = m_contact_whitelist.size();

  // neighbour ping positions
  mat ping_positions = mat(max_size+1, 2);
  ping_positions(0, 0) = 0;
  ping_positions(0, 1) = 0;

  // neighbour plan positions
  mat plan_positions = mat(max_size+1, 2);
  plan_positions(0, 0) = 0;
  plan_positions(0, 1) = 0;

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

    // Relative positions of this pair from the swarm plan (plus the midpoint between the pair)
    double x_rel_plan = m_swarm_plan[neighbour->first].x();
    double y_rel_plan = m_swarm_plan[neighbour->first].y();

    // Relative positions of this pair from acoustic pings (plus the midpoint between the pair)
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
    ping_positions(m_num_contacts+1, 0) = x_rel_ping;
    ping_positions(m_num_contacts+1, 1) = y_rel_ping;
    plan_positions(m_num_contacts+1, 0) = x_rel_plan;
    plan_positions(m_num_contacts+1, 1) = y_rel_plan;
    weights(m_num_contacts+1, 0) = 1.0;

    // Add plan neighbour to its centroid sum
    x_centroid_plan += x_rel_plan;
    y_centroid_plan += y_rel_plan;

    // Add ping neighbour to its centroid sum
    x_centroid_ping += x_rel_ping;
    y_centroid_ping += y_rel_ping;

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
  ping_positions.resize(m_num_contacts+1, 2);
  plan_positions.resize(m_num_contacts+1, 2);
  weights.resize(m_num_contacts+1, 1);

  // Calculate the centroids
  mat centroid_plan = (trans(weights)*plan_positions)/accu(weights);
  mat centroid_ping = (trans(weights)*ping_positions)/accu(weights);

  // Rotation, Translation and Solution
  mat rotation;
  mat translation;
  mat solution;

  // Calculate the rigid transformation
  rigidTransformation2D(plan_positions, ping_positions, weights, centroid_plan, centroid_ping, rotation, translation, solution);

  if (m_display_rigid_registration_points) {
    for (unsigned int i = 0; i < solution.n_rows; i++) {
      m_warning_message << m_us_name << "_rnr_" << i;
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
      m_warning_message << m_us_name << "_rnr_" << m_num_contacts + i;
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
    m_abs_targets.set_label(m_us_name + "_rnr_pnts");
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
    m_abs_targets.set_label(m_us_name + "_rnr_pnts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "invisible");
    m_abs_targets.set_color("edge", "invisible");
    m_abs_targets.set_color("fill", "invisible");
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
  }

  m_rel_target_x = solution(0, 0) * (m_total_weight/m_num_contacts);
  m_rel_target_y = solution(0, 1) * (m_total_weight/m_num_contacts);
  m_rel_target_x_prev = m_rel_target_x;
  m_rel_target_y_prev = m_rel_target_y;
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

void BHV_RigidNeighbourRegistration::rigidTransformation2D(mat p, mat q, mat w, mat cent_p, mat cent_q, mat &R, mat &T, mat &sol)
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
// Global Optimization Functions

double myvfunc(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{
  BHV_AttractionRepulsionOpt::weights_centers_dist *w_c_d = reinterpret_cast<BHV_AttractionRepulsionOpt::weights_centers_dist*>(my_func_data);
  double obj = 0;
  double cen_x = 0.;
  double cen_y = 0.;
  for(int i=0; i < w_c_d->weights.size(); i++){
    obj += 500*w_c_d->weights[i]*(pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 12) - 2*pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 6));
    cen_x += w_c_d->x[i];
    cen_y += w_c_d->y[i];
  }
  obj = obj/w_c_d->weights.size();
  cen_x = cen_x/w_c_d->weights.size();
  cen_y = cen_y/w_c_d->weights.size();
  obj += 0.01*(pow((sqrt(pow((x[0]-cen_x), 2) + pow((x[1]-cen_y), 2))), 2));
  return obj;
}

double myvconstraint(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
    return ((a*x[0] + b) * (a*x[0] + b) * (a*x[0] + b) - x[1]);
}

//---------------------------------------------------------------
// Procedure: optimization()
//   Purpose: Perform the Attraction/Repulsion Non-Linear optimization

void BHV_RigidNeighbourRegistrationOpt::optimization(mat rot, mat tran)
{
  nlopt::opt opt(nlopt::LN_BOBYQA, 6);

  opt.set_min_objective(myvfunc, NULL);

  opt.set_xtol_rel(1e-4);

  std::vector<double> x(6);
  x[0] = rot(0,0); x[1] = rot(0,1); x[2] = rot(0,2); x[3] = rot(0,3); x[4] = tran(0,0); x[5] = tran(1,0);
  double minf;

  try {
    nlopt::result result = opt.optimize(x, minf);
  } catch(exception e) {
    m_warning_message << "optimization failed - returning previous result.";
    postWMessage(m_warning_message.str());
    m_warning_message.clear();
    m_warning_message.str("");
    m_rel_target_x = m_rel_target_x_prev;
    m_rel_target_y = m_rel_target_y_prev;
    return;
  }
  m_rel_target_x = x[0];
  m_rel_target_y = x[1];
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display target points.

void BHV_RigidNeighbourRegistrationOpt::postViewablePoints()
{
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_RigidNeighbourRegistrationOpt::postErasablePoints()
{
}
