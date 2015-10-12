/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Pairs_MapPingFormation.cpp                      */
/*    DATE: March 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "Pairs_MapPingFormation.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

Pairs_MapPingFormation::Pairs_MapPingFormation(IvPDomain gdomain) : Map_PingFormation(gdomain)
{
  setParam("name", "Pairs_MapPingFormation");
  addInfoVars("PING_REPORT");
  m_ping_expiration_distance = 650.;
}


//-----------------------------------------------------------
// Procedure: setParam

bool Pairs_MapPingFormation::setParam(string param, string param_val)
{
  if(Map_PingFormation::setParam(param, param_val))
    return true;

  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "ping_expiration_distance")
  {
    m_ping_expiration_distance = dval;
    return true;
  }

  return false;
}


//-----------------------------------------------------------
// Procedure: onSetParamComplete

void Pairs_MapPingFormation::onSetParamComplete()
{
  Map_PingFormation::onSetParamComplete();

  // The contacts map is now seeing from us
  double us_x = m_map_swarm_elements_position[m_us_name].x();
  double us_y = m_map_swarm_elements_position[m_us_name].y();

  m_map_contacts.erase(m_us_name);
  m_map_swarm_elements_position.erase(m_us_name);

  for(map<string,XYPoint>::iterator contact_position = m_map_swarm_elements_position.begin() ;
      contact_position != m_map_swarm_elements_position.end() ;
      contact_position++)
  {
    contact_position->second.set_vertex(contact_position->second.x() - us_x,
                                        contact_position->second.y() - us_y);
  }

  postIntMessage("SWARM_ELEMENTS", m_map_swarm_elements_position.size());
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Pairs_MapPingFormation::onRunState()
{
  postViewablePoints();

  if(!Map_PingFormation::updatePlatformInfo())
    return 0;

  int nb_subtarget_points = 0;
  double target_x = 0., target_y = 0., total_weights = 0.;
  map<string, int> map_couples_of_contact_analysed;

  // Combining all possible couples:
  for(map<string,PingContact>::iterator first_ping_contact = m_map_contacts.begin() ;
      first_ping_contact != m_map_contacts.end() ;
      first_ping_contact++)
  {
    if(first_ping_contact->second.HasExpired())
      continue;

    for(map<string,PingContact>::iterator second_ping_contact = m_map_contacts.begin() ;
        second_ping_contact != m_map_contacts.end() ;
        second_ping_contact++)
    {
      if(second_ping_contact->second.HasExpired())
        continue;

      if(first_ping_contact->first == second_ping_contact->first) // the first contact itself
        continue;

      if(map_couples_of_contact_analysed.count(first_ping_contact->first + second_ping_contact->first) == 0 &&
          map_couples_of_contact_analysed.count(second_ping_contact->first + first_ping_contact->first) == 0)
      {
        map_couples_of_contact_analysed[first_ping_contact->first + second_ping_contact->first] ++;

        // Theoretical positions
          double xA_th = m_map_swarm_elements_position[first_ping_contact->first].x();
          double yA_th = m_map_swarm_elements_position[first_ping_contact->first].y();

          double xB_th = m_map_swarm_elements_position[second_ping_contact->first].x();
          double yB_th = m_map_swarm_elements_position[second_ping_contact->first].y();

          double xC_th = (xA_th + xB_th) / 2.;
          double yC_th = (yA_th + yB_th) / 2.;

          double dist_from_midpoint_th = hypot(xC_th, yC_th);
          double angle_from_midpoint_th;

        // Angle ACM (M = us)
          angle_from_midpoint_th = relAng(xC_th, yC_th, 0., 0.) - relAng(xC_th, yC_th, xB_th, yB_th);

        // True positions
          double xA_tr = first_ping_contact->second.GetRelativeX();
          double yA_tr = first_ping_contact->second.GetRelativeY();

          double xB_tr = second_ping_contact->second.GetRelativeX();
          double yB_tr = second_ping_contact->second.GetRelativeY();

          double xC_tr = (xA_tr + xB_tr) / 2.;
          double yC_tr = (yA_tr + yB_tr) / 2.;

          double dist_from_midpoint_tr = hypot(xC_tr, yC_tr);

        if(dist_from_midpoint_tr > m_ping_expiration_distance)
          continue;

        // Projection
          double subtarget_x, subtarget_y;
          projectPoint(angle360(angle_from_midpoint_th + relAng(xA_tr, yA_tr, xB_tr, yB_tr)), dist_from_midpoint_th,
                        xC_tr, yC_tr,
                        subtarget_x, subtarget_y);

        // Lapsing
          double mean_lapsing = (first_ping_contact->second.lapsing() + second_ping_contact->second.lapsing()) / 2.;
          double weight_lapsing = 1. - (mean_lapsing / m_ping_expiration_delay);

        // Distance
          double weight_distance = 1. - (dist_from_midpoint_tr / m_ping_expiration_distance);

        target_x += weight_lapsing * weight_distance * subtarget_x;
        target_y += weight_lapsing * weight_distance * subtarget_y;
        total_weights += weight_lapsing * weight_distance;
        nb_subtarget_points ++;
      }
    }
  }

  postMessage("TOTAL_WEIGHTS", total_weights);
  postMessage("NB_SUBTARGET_POINTS", nb_subtarget_points);

  m_relative_target_point.set_vertex(target_x / total_weights, target_y / total_weights);
  filterCentroidTargetpoint();

  return Map_PingFormation::onRunState();
}


//-----------------------------------------------------------
// Procedure: postViewablePoints

void Pairs_MapPingFormation::postViewablePoints()
{
  Map_PingFormation::postViewablePoints();
}


//-----------------------------------------------------------
// Procedure: postErasablePoints

void Pairs_MapPingFormation::postErasablePoints()
{
  Map_PingFormation::postErasablePoints();
}


//-----------------------------------------------------------
// Procedure: onRunToIdleState

void Pairs_MapPingFormation::onRunToIdleState()
{
  postMessage("PURSUIT", 0);
  postErasablePoints();
}
