/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForceWaypoint.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include <cmath>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_DriftingForceWaypoint.h"
#include <iostream>

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_DriftingForceWaypoint::BHV_DriftingForceWaypoint(IvPDomain domain) :
  IvPBehavior(domain), BHV_DriftingForce(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_DriftingForceWaypoint");

  m_curr_wpt_x = 0.;
  m_curr_wpt_y = 0.;
  m_dist = 50.;

  addInfoVars("FORCE_DIST");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_DriftingForceWaypoint::setParam(string param, string val)
{
  if (BHV_DriftingForce::setParam(param, val))
    return true;

  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());

  if ((param == "point_x")  && (isNumber(val))) {
    m_wpt_x = double_val;
    return(true);
  } else if ((param == "point_y")  && (isNumber(val))) {
    m_wpt_y = double_val;
    return(true);
  } else if ((param == "force")  && (isNumber(val))) {
    m_force = double_val;
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

void BHV_DriftingForceWaypoint::onSetParamComplete()
{
  BHV_DriftingForce::onSetParamComplete();
}

IvPFunction* BHV_DriftingForceWaypoint::onRunState()
{
  if (!BHV_DriftingForce::updateInfoIn())
    return NULL;

  bool ok1;
  m_dist = getBufferDoubleVal("FORCE_DIST", ok1);
  if (!ok1) m_dist = 50.0;

  if ((m_wpt_x != m_curr_wpt_x) || (m_wpt_y != m_curr_wpt_y)) {
    postWMessage("WAYPOINT ADDED!!!");
    m_curr_wpt_x = m_wpt_x;
    m_curr_wpt_y = m_wpt_y;
    XYPoint pt;
    pt.set_vertex(m_wpt_x, m_wpt_y);
    m_wpts.push_front(pt);
  }

  ostringstream strs;
  int counter = 0;
  double total_force_x = 0.;
  double total_force_y = 0.;
  for (list<XYPoint>::iterator it = m_wpts.begin(); it != m_wpts.end(); ++it) {
    double wpt_x, wpt_y;
    wpt_x = it->x();
    wpt_y = it->y();
    m_force_heading = relAng(m_nav_x, m_nav_y, wpt_x, wpt_y);
    double dist = hypot(wpt_x - m_nav_x, wpt_y - m_nav_y);
    strs << m_us_name << "_" << counter;
    m_circle.set_label(strs.str());
    strs.clear();
    strs.str("");
    m_circle.set_active("true");
    m_circle.set_color("label", "red");
    m_circle.set_color("edge", "red");
    m_circle.set_color("fill", "red");
    m_circle.setRad(m_dist);
    m_circle.set_vertex_size(2);
    m_circle.set_edge_size(1);
    m_circle.set_transparency(0.1);
    m_circle.setX(wpt_x);
    m_circle.setY(wpt_y);
    postMessage("VIEW_CIRCLE", m_circle.get_spec());
    strs << m_us_name << "__" << counter;
    m_circle2.set_label(strs.str());
    strs.clear();
    strs.str("");
    m_circle2.set_active("true");
    m_circle2.set_color("label", "yellow");
    m_circle2.set_color("edge", "yellow");
    m_circle2.set_color("fill", "yellow");
    m_circle2.setRad(m_dist/2);
    m_circle2.set_vertex_size(2);
    m_circle2.set_edge_size(1);
    m_circle2.set_transparency(0.1);
    m_circle2.setX(wpt_x);
    m_circle2.setY(wpt_y);
    postMessage("VIEW_CIRCLE", m_circle2.get_spec());
    attractionConstantRepulsionUnbounded(dist);
    double force_x;
    double force_y;
    projectPoint(m_force_heading, m_force, 0., 0., force_x, force_y);
    total_force_x += force_x;
    total_force_y += force_y;
    counter++;
  }

  BHV_DriftingForce::setForce(total_force_x, total_force_y);

  return BHV_DriftingForce::onRunState();
}

//maybe add a piecewise attraction/repulsion?
void BHV_DriftingForceWaypoint::attractionConstantRepulsionUnbounded(double r)
{
  // force = -r*(100/r - 100*50/r^2)
  double a, b, x, y, force;
  int x_pow, y_pow, diff_pow;
  x_pow = 1;
  y_pow = 12;
  diff_pow = y_pow - x_pow;
  x = pow(r,x_pow);
  y = pow(r,y_pow);
  a = 100;
  b = 100*pow(50,diff_pow);
  m_force = -r*(a/x - b/y);
  if (m_force < 0) {
    m_force = -m_force;
  } else {
    m_force_heading = angle360(m_force_heading + 180.0);
  }
}

void BHV_DriftingForceWaypoint::attractionLinearRepulsionUnbounded(double r)
{
  // force = -r*(100/r - 100*50/r^2)
  double r_sq = pow(r,2);
  double crossing_sq = pow(50,2);
  m_force = -r*(2 - (2*crossing_sq)/(r_sq));
  if (m_force < 0) {
    m_force = -m_force;
  } else {
    m_force_heading = angle360(m_force_heading + 180.0);
  }
}

void BHV_DriftingForceWaypoint::attractionLinearRepulsionBounded(double r)
{
  // force = -r*(100/r - 100*50/r^2)
  double r_sq = pow(r,2);
  double crossing_sq = pow(50,2);
  double exponent = exp(-1);
  m_force = -r*(1 - (1/exponent)*exp(-(r_sq/crossing_sq)));
  if (m_force < 0) {
    m_force = -m_force;
  } else {
    m_force_heading = angle360(m_force_heading + 180.0);
  }
}
