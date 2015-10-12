/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsionOpt.cpp                  */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <cmath>
#include <limits>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_AttractionRepulsionOpt.h"
#include <nlopt.hpp>
#include "Geometry/IncrementalConvexHull.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_AttractionRepulsionOpt::BHV_AttractionRepulsionOpt(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_AttractionRepulsionOpt");

  // Initialize config variables
  m_contact_rangeout = 650.;
  m_force_rangeout = 650.;
  m_weight_targets = "";
  m_separation_distance = 300.;
  m_max_num_neighbours = 3;
  m_target_avg_length = 30;

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
  m_num_nbrs = 0;
  m_num_nbrs_prev = 0;
  m_warning_message.clear();
  m_warning_message.str("");
  m_rel_target_x_deque.clear();
  m_rel_target_y_deque.clear();
  m_deque_size = m_target_avg_length;
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_AttractionRepulsionOpt::setParam(string param, string val)
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
  } else if ((param == "force_rangeout") && non_neg_number) {
    m_force_rangeout = double_val;
    return(true);
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
  } else if ((param == "separation_distance") && non_neg_number) {
    m_separation_distance = double_val;
    return(true);
  } else if ((param == "max_num_neighbours") && non_neg_number) {
    m_max_num_neighbours = (int)double_val;
    return(true);
  } else if ((param == "target_avg_length") && non_neg_number) {
    m_target_avg_length = (int)double_val;
    m_deque_size = m_target_avg_length;
    return(true);
  } else if ((param == "display_neighbour_hull")) {
    return(setBooleanOnString(m_display_neighbour_hull, val));
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_AttractionRepulsionOpt::onSetParamComplete()
{
  BHV_DriftingTarget::onSetParamComplete();
  BHV_AcousticPingPlanner::onSetParamComplete();

  m_abs_target.set_color("label", "invisible");
  m_abs_target.set_color("vertex", "magenta");
  m_abs_target.set_vertex_size(10);
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_AttractionRepulsionOpt::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_AttractionRepulsionOpt::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_AttractionRepulsionOpt::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_AttractionRepulsionOpt::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_AttractionRepulsionOpt::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_AttractionRepulsionOpt::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
//   Golbal comparison function to sort our vector of dist, x, y.

bool pairCompare(const std::pair< double, std::pair<double, double> >& firstElem, const std::pair< double, std::pair<double, double> >& secondElem)
{
  return firstElem.first < secondElem.first;
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_AttractionRepulsionOpt::onRunState()
{
  if (!BHV_DriftingTarget::updateInfoIn())
    return NULL;
  if (!BHV_AcousticPingPlanner::updateInfoIn())
    return NULL;

  BHV_AcousticPingPlanner::onRunState();

  m_distance_weight = 0.;
  m_pingtime_weight = 0.;
  m_total_weight = 0.;
  m_num_contacts = 0;
  m_warning_message.clear();
  m_warning_message.str("");
  m_weights_centers_dist.weights.clear();
  m_weights_centers_dist.x.clear();
  m_weights_centers_dist.y.clear();
  m_weights_centers_dist.dist = m_separation_distance;
  m_centroid_x = 0.;
  m_centroid_y = 0.;
  m_dist_x_y.clear();

  // loop through all possible pairs of neighbours in the formation whitelist (don't need the plan, just list of neighbours for acoustics)
  for (map<string,PingContact>::iterator neighbour = m_contact_whitelist.begin(); neighbour != m_contact_whitelist.end(); ++neighbour) {  // loop through all neighbours in the whitelist
    if (neighbour->second.GetExpired()) {
//      m_warning_message << neighbour->first << " neighbour has not been 'seen' acoustically for more than (specified) " << BHV_AcousticPingPlanner::m_contact_timeout << " seconds.";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      continue;
    }

    // Relative position of this neighbour from acoustic pings
    double x_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
    double y_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

    double dist_rel_ping = hypot(x_rel_ping, y_rel_ping);

    if (dist_rel_ping > m_contact_rangeout) {
//      m_warning_message << neighbour->first << " neighbour is outside of CONTACT rangeout distance " << m_contact_rangeout << ".";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      continue;
    } else if (dist_rel_ping > m_force_rangeout) {
//      m_warning_message << neighbour->first << " neighbour is outside of FORCE rangeout distance " << m_force_rangeout << ". Ignoring.";
//      postWMessage(m_warning_message.str());
//      m_warning_message.clear();
//      m_warning_message.str("");
      continue;
    }

    m_dist_x_y.push_back(std::make_pair(dist_rel_ping, std::make_pair(x_rel_ping, y_rel_ping)));

    // Calculate centroids
    m_centroid_x += x_rel_ping;
    m_centroid_y += y_rel_ping;

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

    // Accumulate total weight
    m_total_weight += m_distance_weight*m_pingtime_weight;

    m_weights_centers_dist.weights.push_back(1-(m_pingtime_weight*m_distance_weight));
    m_weights_centers_dist.x.push_back(x_rel_ping);
    m_weights_centers_dist.y.push_back(y_rel_ping);

    m_num_contacts++;
  }

  vector<double> temp_x;
  vector<double> temp_y;
  vector<double> temp_w;
  sort(m_dist_x_y.begin(), m_dist_x_y.end(), pairCompare);

  /** INITIAL **/
//  for (unsigned int i = 0; i < m_dist_x_y.size(); i++) {
//    if (i >= m_max_num_neighbours) break;
//    temp_x.push_back(m_dist_x_y[i].second.first);
//    temp_y.push_back(m_dist_x_y[i].second.second);
//    temp_w.push_back(1.0);
//  }

  /** ALTERNATIVE **/
  double second_nbr_dist = std::numeric_limits<double>::max();
  double second_nbr_x = 0;
  double second_nbr_y = 0;
  for (unsigned int i = 0; i < m_dist_x_y.size(); i++) {
    if (i == 0) {
      temp_x.push_back(m_dist_x_y[i].second.first);
      temp_y.push_back(m_dist_x_y[i].second.second);
      temp_w.push_back(1.0);
    } else {
      double curr_dist = hypot(temp_x[0]-m_dist_x_y[i].second.first, temp_y[0]-m_dist_x_y[i].second.second) + m_dist_x_y[i].first;
      if (curr_dist < second_nbr_dist) {
        second_nbr_dist = curr_dist;
        second_nbr_x = m_dist_x_y[i].second.first;
        second_nbr_y = m_dist_x_y[i].second.second;
      }
    }
  }
  if (second_nbr_dist < std::numeric_limits<double>::max()) {
    temp_x.push_back(second_nbr_x);
    temp_y.push_back(second_nbr_y);
    temp_w.push_back(1.0);
  }
  m_num_nbrs = temp_x.size();

  m_weights_centers_dist.x = temp_x;
  m_weights_centers_dist.y = temp_y;
  m_weights_centers_dist.weights = temp_w;

  m_centroid_x = m_centroid_x/(m_num_contacts + 1);
  m_centroid_y = m_centroid_y/(m_num_contacts + 1);

  m_weights_centers_dist.centroid_x = m_centroid_x;
  m_weights_centers_dist.centroid_y = m_centroid_y;

  optimization();

  m_rel_target_x = m_rel_target_x*m_total_weight/m_num_contacts;
  m_rel_target_y = m_rel_target_y*m_total_weight/m_num_contacts;
  if (m_rel_target_x_deque.size() > m_deque_size) {
    m_rel_target_x_deque.pop_front();
    m_rel_target_y_deque.pop_front();
  }
  m_rel_target_x_deque.push_back(m_rel_target_x);
  m_rel_target_y_deque.push_back(m_rel_target_y);
  m_rel_target_x = 0.;
  m_rel_target_y = 0.;
  for (unsigned int i = 0; i < m_rel_target_x_deque.size(); i++) {
    m_rel_target_x += m_rel_target_x_deque[i];
    m_rel_target_y += m_rel_target_y_deque[i];
  }
  m_rel_target_x = m_rel_target_x/m_deque_size;
  m_rel_target_y = m_rel_target_y/m_deque_size;

  m_rel_target_x_prev = m_rel_target_x;
  m_rel_target_y_prev = m_rel_target_y;

  IncrementalConvexHull convex_hull;
  if (m_display_neighbour_hull) {
    convex_hull.addPoint(BHV_DriftingTarget::m_nav_x, BHV_DriftingTarget::m_nav_y);
    for (unsigned int i = 0; i < m_weights_centers_dist.x.size(); i++) {
      srand(time(NULL) + i);
      double x_delta = ((double)rand()/(double)RAND_MAX)*0.1 - 0.05;
      double y_delta = ((double)rand()/(double)RAND_MAX)*0.1 - 0.05;
      // NOTE: SOMETHING IS WRONG WITH THE CHECKING OF POINTS WITHIN THE EXISTING CONVEX HULL (WHEN ANOTHER POINT IS AT THE EXACT INTERSECTION OF TWO EXISTING LINES) - THIS IS A HACKY WARKAROUND
      convex_hull.addPoint(m_weights_centers_dist.x[i] + BHV_DriftingTarget::m_nav_x + x_delta, m_weights_centers_dist.y[i] + BHV_DriftingTarget::m_nav_y + y_delta);
    }
  }
  if (m_num_nbrs+1 >= 3 && m_display_neighbour_hull) {
    // a polygon is only valid when there is more than 2 pair of contacts (three vertices needed)
    m_abs_targets = convex_hull.getConvexHullAsPolygon();
    m_abs_targets.set_label(m_us_name + "_aro_pnts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "magenta");
    m_abs_targets.set_color("edge", "magenta");
    m_abs_targets.set_color("fill", "white");
    m_abs_targets.set_transparency(0.1);
    m_abs_targets.set_vertex_size(3);
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
    convex_hull.deleteAllPoints();
  } else if (m_display_neighbour_hull && (m_num_nbrs < m_num_nbrs_prev)) {
    // remove previously drawn polygons - this occurs dynamically when there is only one pair of contacts left
    m_abs_targets.clear();
    m_abs_targets.add_vertex(0, 0);
    m_abs_targets.add_vertex(1, 0);
    m_abs_targets.add_vertex(1, 1);
    m_abs_targets.set_label(m_us_name + "_aro_pnts");
    m_abs_targets.set_color("label", "invisible");
    m_abs_targets.set_color("vertex", "invisible");
    m_abs_targets.set_color("edge", "invisible");
    m_abs_targets.set_color("fill", "invisible");
    postMessage("VIEW_POLYGON", m_abs_targets.get_spec());
  }

   m_num_nbrs_prev = m_num_nbrs;

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
// Global Optimization Functions

double myvfunc(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data)
{
  BHV_AttractionRepulsionOpt::weights_centers_dist *w_c_d = reinterpret_cast<BHV_AttractionRepulsionOpt::weights_centers_dist*>(my_func_data);
  double obj = 0;
  double cen_x = 0.;
  double cen_y = 0.;
  for(int i=0; i < w_c_d->weights.size(); i++){
//    obj += 500*w_c_d->weights[i]*(pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 12) - 2*pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 6));
//    obj += 500*w_c_d->weights[i]*(pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 12) - 2*pow((w_c_d->dist/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 6) + pow(((sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2))) - w_c_d->dist), 2));
    obj += ((pow(w_c_d->dist, 3)/2)/pow(((sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 2) + (((sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2))) - (3*w_c_d->dist))));
    cen_x += w_c_d->x[i];
    cen_y += w_c_d->y[i];
  }
  obj = obj/w_c_d->weights.size();
  cen_x = cen_x/w_c_d->weights.size();
  cen_y = cen_y/w_c_d->weights.size();
//  obj += 0.01*(pow((sqrt(pow((x[0]-cen_x), 2) + pow((x[1]-cen_y), 2))), 2));
  obj += 1e-5*(pow((sqrt(pow((x[0]-w_c_d->centroid_x), 2) + pow((x[1]-w_c_d->centroid_y), 2))), 2));  //need this factor to keep different parts of the formation together -> when using just two neighbours, its possible for the formation to 'split'
  return obj;
}

//---------------------------------------------------------------
// Procedure: optimization()
//   Purpose: Perform the Attraction/Repulsion Non-Linear optimization

void BHV_AttractionRepulsionOpt::optimization()
{
  nlopt::opt opt(nlopt::LN_BOBYQA, 2);

//  stringstream warning_message;
//  for (unsigned int i = 0; i<m_weights_centers_dist.x.size(); i++) {
//    warning_message << m_weights_centers_dist.x[i] << " ";
//  }
//  postWMessage(warning_message.str());

  opt.set_min_objective(myvfunc, &m_weights_centers_dist);

  opt.set_xtol_rel(1e-4);

  std::vector<double> x(2);
  x[0] = 0.; x[1] = 0.;
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

void BHV_AttractionRepulsionOpt::postViewablePoints()
{
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_AttractionRepulsionOpt::postErasablePoints()
{
}
