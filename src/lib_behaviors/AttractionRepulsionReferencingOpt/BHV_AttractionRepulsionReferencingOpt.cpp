/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsionReferencingOpt.cpp       */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <cmath>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_AttractionRepulsionReferencingOpt.h"
#include <nlopt.hpp>

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_AttractionRepulsionReferencingOpt::BHV_AttractionRepulsionReferencingOpt(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingTarget(domain), BHV_AcousticPingPlanner(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_AttractionRepulsionReferencingOpt");

  // Initialize config variables
  m_contact_rangeout = 650.;
  m_force_rangeout = 450.;
  m_weight_targets = "";

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

bool BHV_AttractionRepulsionReferencingOpt::setParam(string param, string val)
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
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_AttractionRepulsionReferencingOpt::onSetParamComplete()
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

void BHV_AttractionRepulsionReferencingOpt::onHelmStart()
{
  BHV_DriftingTarget::onHelmStart();
  BHV_AcousticPingPlanner::onHelmStart();
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_AttractionRepulsionReferencingOpt::onIdleState()
{
  BHV_DriftingTarget::onIdleState();
  BHV_AcousticPingPlanner::onIdleState();
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_AttractionRepulsionReferencingOpt::onCompleteState()
{
  BHV_DriftingTarget::onCompleteState();
  BHV_AcousticPingPlanner::onCompleteState();
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_AttractionRepulsionReferencingOpt::postConfigStatus()
{
  BHV_DriftingTarget::postConfigStatus();
  BHV_AcousticPingPlanner::postConfigStatus();
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_AttractionRepulsionReferencingOpt::onIdleToRunState()
{
  BHV_DriftingTarget::onIdleToRunState();
  BHV_AcousticPingPlanner::onIdleToRunState();
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_AttractionRepulsionReferencingOpt::onRunToIdleState()
{
  BHV_DriftingTarget::onRunToIdleState();
  BHV_AcousticPingPlanner::onRunToIdleState();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_AttractionRepulsionReferencingOpt::onRunState()
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
  m_weights_centers_dists.weights.clear();
  m_weights_centers_dists.x.clear();
  m_weights_centers_dists.y.clear();
  m_weights_centers_dists.dists.clear();

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
    double x_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->x() - BHV_DriftingTarget::m_nav_x;
    double y_rel_ping = neighbour->second.GetAbsoluteFilteredPointSupposed()->y() - BHV_DriftingTarget::m_nav_y;

    // Relative distance to this neighbour from acoustic pings
    double dist_rel_ping = hypot(x_rel_ping, y_rel_ping);
    double heading_rel_ping = relAng(0, 0, x_rel_ping, y_rel_ping);

    // Relative positions of this neighbour from the swarm plan
    double x_rel_plan = m_swarm_plan[neighbour->first].x();
    double y_rel_plan = m_swarm_plan[neighbour->first].y();

    // Relative distance to this neighbour from the swarm plan
    double dist_rel_plan = hypot(x_rel_plan, y_rel_plan);

    if (dist_rel_ping > m_contact_rangeout) {
      m_warning_message << neighbour->first << " neighbour is outside of CONTACT rangeout distance " << m_contact_rangeout << ".";
      postWMessage(m_warning_message.str());
      m_warning_message.clear();
      m_warning_message.str("");
      continue;
    } else if (dist_rel_ping > m_force_rangeout) {
      m_warning_message << neighbour->first << " neighbour is outside of FORCE rangeout distance " << m_force_rangeout << ". Ignoring.";
      postWMessage(m_warning_message.str());
      m_warning_message.clear();
      m_warning_message.str("");
      continue;
    }

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

    m_weights_centers_dists.weights.push_back(m_pingtime_weight*m_distance_weight);
    m_weights_centers_dists.x.push_back(x_rel_ping);
    m_weights_centers_dists.y.push_back(y_rel_ping);
    m_weights_centers_dists.dists.push_back(dist_rel_plan);

    m_num_contacts++;
  }

  optimization();

  m_rel_target_x = m_rel_target_x*m_total_weight/m_num_contacts;
  m_rel_target_y = m_rel_target_y*m_total_weight/m_num_contacts;
  m_rel_target_x_prev = m_rel_target_x;
  m_rel_target_y_prev = m_rel_target_y;

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
  BHV_AttractionRepulsionReferencingOpt::weights_centers_dists *w_c_d = reinterpret_cast<BHV_AttractionRepulsionReferencingOpt::weights_centers_dists*>(my_func_data);
  double obj = 0;
  double cen_x = 0.;
  double cen_y = 0.;
  for(int i=0; i < w_c_d->weights.size(); i++){
//    obj += pow((w_c_d->dists[i]/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 3) - 2*pow((w_c_d->dists[i]/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 1);
    obj += 500*w_c_d->weights[i]*(pow((w_c_d->dists[i]/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 12) - 2*pow((w_c_d->dists[i]/(sqrt(pow((x[0]-w_c_d->x[i]), 2) + pow((x[1]-w_c_d->y[i]), 2)))), 6));
    cen_x += w_c_d->x[i];
    cen_y += w_c_d->y[i];
  }
  obj = obj/w_c_d->weights.size();
  cen_x = cen_x/w_c_d->weights.size();
  cen_y = cen_y/w_c_d->weights.size();
  obj += 0.01*(pow((sqrt(pow((x[0]-cen_x), 2) + pow((x[1]-cen_y), 2))), 2));
  return obj;
}

//---------------------------------------------------------------
// Procedure: optimization()
//   Purpose: Perform the Attraction/Repulsion Non-Linear optimization

void BHV_AttractionRepulsionReferencingOpt::optimization()
{
  nlopt::opt opt(nlopt::LN_BOBYQA, 2);

  opt.set_min_objective(myvfunc, &m_weights_centers_dists);

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

void BHV_AttractionRepulsionReferencingOpt::postViewablePoints()
{
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase target points.

void BHV_AttractionRepulsionReferencingOpt::postErasablePoints()
{
}
