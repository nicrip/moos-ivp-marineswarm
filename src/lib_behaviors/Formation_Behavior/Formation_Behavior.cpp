/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Formation_Behavior.cpp                          */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#ifdef _WIN32
  #pragma warning(disable : 4786)
  #pragma warning(disable : 4503)
#endif

#include "Formation_Behavior.h"
#include "BuildUtils.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"

using namespace std;

int Formation_Behavior::nb_formation_behaviors_ = 0;
double Formation_Behavior::total_time_drifting_ = 0;
double Formation_Behavior::total_time_repositioning_ = 0;
double Formation_Behavior::drifting_date_ = MOOSTime();
double Formation_Behavior::repositioning_date_ = MOOSTime();
int Formation_Behavior::nb_active_repositioning_behaviors_ = 0;


//-----------------------------------------------------------
// Procedure: Constructor

Formation_Behavior::Formation_Behavior(IvPDomain gdomain) : IvPBehavior(gdomain)
{
  setParam("name", "Formation_Behavior");
  m_domain = subDomain(m_domain, "course,speed");
  this->setParam("descriptor", "(d)formation_behavior");
  this->setParam("build_info", "uniform_piece=discrete@course:3,speed:2");
  this->setParam("build_info", "uniform_grid =discrete@course:9,speed:6");

  // If outside this radius to the contact, head to ext_radius ahead of target point
  m_int_radius  = 10.;
  // If in this range to contact and ahead of it, slow down
  m_ext_radius  = 30.;

  // Ownship
  m_nav_x       = 0.;
  m_nav_y       = 0.;
  m_heading     = 0.; // The current heading

  addInfoVars("NAV_DEPTH, NAV_HEADING");
  addInfoVars("NAV_X, NAV_Y"); // only used for display

  m_max_speed                 = 1.0;
  m_drift_passively           = false;
  m_display_targetpoints      = false;
  m_display_radii             = false;
  m_display_statistics        = false;
  m_decaying_speed_ratio      = true;
  m_filter_size_targetpoints  = 10;

  m_relative_target_point.set_vertex(0., 0.);

  Formation_Behavior::increaseActiveRepositioningBehaviors();
  nb_formation_behaviors_ ++;
  postIntMessage("NB_FORMATION_BHV", nb_formation_behaviors_);
}


//-----------------------------------------------------------
// Procedure: updatePlatformInfo

bool Formation_Behavior::updatePlatformInfo()
{
  bool ok;

  m_heading = getBufferDoubleVal("NAV_HEADING", ok);
  if(!ok)
  {
    string e_message = "updateInfo: NAV_HEADING info not found.";
    postEMessage(e_message);
    cout << e_message << endl;
    return false;
  }

  m_nav_depth = getBufferDoubleVal("NAV_DEPTH", ok);
  if(!ok)
  {
    string e_message = "updateInfo: NAV_DEPTH info not found.";
    postEMessage(e_message);
    cout << e_message << endl;
    return false;
  }

  m_nav_x = getBufferDoubleVal("NAV_X", ok);
  if(!ok)
  {
    string e_message = "updateInfo: NAV_X info not found.";
    postEMessage(e_message);
    cout << e_message << endl;
    return false;
  }

  m_nav_y = getBufferDoubleVal("NAV_Y", ok);
  if(!ok)
  {
    string e_message = "updateInfo: NAV_Y info not found.";
    postEMessage(e_message);
    cout << e_message << endl;
    return false;
  }

  return true;
}


//-----------------------------------------------------------
// Procedure: setParam

bool Formation_Behavior::setParam(string param, string param_val)
{
  if(IvPBehavior::setParam(param, param_val))
    return true;

  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "int_radius" && non_neg_number)
  {
    m_int_radius = dval;
    return true;
  }

  if(param == "ext_radius" && non_neg_number)
  {
    m_ext_radius = dval;
    return true;
  }

  if(param == "max_speed" && non_neg_number)
  {
    m_max_speed = dval;
    return true;
  }

  if(param == "display_targetpoints")
    return setBooleanOnString(m_display_targetpoints, param_val);

  if(param == "display_radii")
    return setBooleanOnString(m_display_radii, param_val);

  if(param == "display_statistics")
    return setBooleanOnString(m_display_statistics, param_val);

  if(param == "decaying_speed_ratio")
    return setBooleanOnString(m_decaying_speed_ratio, param_val);

  if(param == "filter_size_targetpoints")
  {
    m_filter_size_targetpoints = dval;
    return true;
  }

  return false;
}


//-----------------------------------------------------------
// Procedure: onSetParamComplete

void Formation_Behavior::onSetParamComplete()
{
  // Targetpoint
  m_absolute_target_point.set_label(m_us_name + "_targetpt");
  m_absolute_target_point.set_active("false");
  m_absolute_target_point.set_color("label", "yellow");

  // Int radius
  m_view_int_radius.set_label("int" + toupper(m_us_name).substr(0, 1));
  m_view_int_radius.set_active("false");
  m_view_int_radius.set_color("label", "red");
  m_view_int_radius.set_color("edge", "red");
  m_view_int_radius.set_color("fill", "red");
  m_view_int_radius.setRad(m_int_radius);
  m_view_int_radius.set_vertex_size(2);
  m_view_int_radius.set_edge_size(1);
  m_view_int_radius.set_transparency(0.3);

  // Ext radius
  m_view_ext_radius.set_label("ext" + toupper(m_us_name).substr(0, 1));
  m_view_ext_radius.set_active("false");
  m_view_ext_radius.set_color("label", "orange");
  m_view_ext_radius.set_color("edge", "orange");
  m_view_ext_radius.set_color("fill", "orange");
  m_view_ext_radius.setRad(m_ext_radius);
  m_view_ext_radius.set_vertex_size(2);
  m_view_ext_radius.set_edge_size(1);
  m_view_ext_radius.set_transparency(0.3);
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *Formation_Behavior::onRunState()
{
  postViewablePoints();
  postMessage("PURSUIT", 1);

  // We consider the ownship placed at (0,0)
  double dist_ownship_targetpoint = hypot(m_relative_target_point.x(), m_relative_target_point.y());
  double angle_ownship_targetpoint = relAng(0., 0., m_relative_target_point.x(), m_relative_target_point.y());
  postMessage("WP_DISTANCE", dist_ownship_targetpoint);
  postMessage("WP_HEADING", angle_ownship_targetpoint);
  postMessage("WP_RELATIVE_X", m_relative_target_point.x());
  postMessage("WP_RELATIVE_Y", m_relative_target_point.y());

  IvPFunction *ipf = 0;

  if(dist_ownship_targetpoint > m_int_radius)
  {
    if(dist_ownship_targetpoint < m_ext_radius)
      ipf = IvPFunctionWhenInsideExtRadius(angle_ownship_targetpoint, dist_ownship_targetpoint);

    else
      ipf = IvPFunctionWhenOutsideExtRadius(angle_ownship_targetpoint);
  }

  else
    ipf = IvPFunctionWhenInsideIntRadius(angle_ownship_targetpoint);

  if(dist_ownship_targetpoint < m_int_radius && !m_drift_passively)
  {
    m_drift_passively = true;
    Formation_Behavior::decreaseActiveRepositioningBehaviors();
  }

  else if(dist_ownship_targetpoint > m_ext_radius && m_drift_passively)
  {
    m_drift_passively = false;
    Formation_Behavior::increaseActiveRepositioningBehaviors();
  }

  updateStatistics();

  if(ipf)
  {
    ipf->getPDMap()->normalize(0., 100.);

    if(m_drift_passively)
      ipf->setPWT(0.);

    else
      ipf->setPWT(m_priority_wt);
  }

  return ipf;
}


//-----------------------------------------------------------
// Procedure: IvPFunctionWhenInsideIntRadius

IvPFunction *Formation_Behavior::IvPFunctionWhenInsideIntRadius(double heading)
{
  postMessage("REGION", "Inside int_radius");

  // Heading
    ZAIC_PEAK hdg_zaic(m_domain, "course");
    // summit, pwidth, bwidth, delta, minutil, maxutil
    hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
    hdg_zaic.setValueWrap(true);
    IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
    ZAIC_PEAK spd_zaic(m_domain, "speed");
    // summit, pwidth, bwidth, delta, minutil, maxutil
    spd_zaic.setParams(0., 0.1 * m_max_speed, 0.3 * m_max_speed, 60., 0., 100.);
    IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
    OF_Coupler coupler;
    return coupler.couple(hdg_ipf, spd_ipf);
}


//-----------------------------------------------------------
// Procedure: IvPFunctionWhenInsideExtRadius

IvPFunction *Formation_Behavior::IvPFunctionWhenInsideExtRadius(double heading, double dist_from_targetpoint)
{
  postMessage("REGION", "Inside ext_radius");

  // Heading
    ZAIC_PEAK hdg_zaic(m_domain, "course");
    // summit, pwidth, bwidth, delta, minutil, maxutil
    hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
    hdg_zaic.setValueWrap(true);
    IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
    ZAIC_PEAK spd_zaic(m_domain, "speed");
    spd_zaic.setValueWrap(false);

    double speed_ratio = 1.;

    if(m_decaying_speed_ratio)
      speed_ratio = (dist_from_targetpoint - m_int_radius) / (m_ext_radius - m_int_radius);

    // summit, pwidth, bwidth, delta, minutil, maxutil
    spd_zaic.setParams(max(0.1, speed_ratio * m_max_speed),   // summit
                       0.3 * m_max_speed,                     // pwidth
                       0.1 * m_max_speed,                     // bwidth
                       50.,                                   // delta
                       0.,                                    // minutil
                       100.);                                 // maxutil
    IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
    OF_Coupler coupler;
    return coupler.couple(hdg_ipf, spd_ipf);
}


//-----------------------------------------------------------
// Procedure: IvPFunctionWhenOutsideExtRadius

IvPFunction *Formation_Behavior::IvPFunctionWhenOutsideExtRadius(double heading)
{
  postMessage("REGION", "Outside ext_radius");

  // Heading
    ZAIC_PEAK hdg_zaic(m_domain, "course");
    // summit, pwidth, bwidth, delta, minutil, maxutil
    hdg_zaic.setParams(heading, 90., 75., 20., 0., 100.);
    hdg_zaic.setValueWrap(true);
    IvPFunction *hdg_ipf = hdg_zaic.extractIvPFunction();

  // Speed
    ZAIC_PEAK spd_zaic(m_domain, "speed");
    // summit, pwidth, bwidth, delta, minutil, maxutil
    spd_zaic.setParams(m_max_speed, 0.2 * m_max_speed, 0.4 * m_max_speed, 50., 0., 100.);
    IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();

  // Coupling
    OF_Coupler coupler;
    return coupler.couple(hdg_ipf, spd_ipf);
}


//-----------------------------------------------------------
// Procedure: onRunToIdleState

void Formation_Behavior::onRunToIdleState()
{
  postMessage("PURSUIT", 0);
  postErasablePoints();
}


//-----------------------------------------------------------
// Procedure: postViewablePoints

void Formation_Behavior::postViewablePoints()
{
  updateAbsoluteTargetPoint(!m_drift_passively);

  if(m_display_targetpoints)
    postMessage("VIEW_POINT", m_absolute_target_point.get_spec());

  if(m_display_radii)
  {
    postMessage("VIEW_CIRCLE", m_view_ext_radius.get_spec());
    postMessage("VIEW_CIRCLE", m_view_int_radius.get_spec());
  }
}


//-----------------------------------------------------------
// Procedure: postErasablePoints

void Formation_Behavior::postErasablePoints()
{
  updateAbsoluteTargetPoint(false);

  if(m_display_targetpoints)
    postMessage("VIEW_POINT", m_absolute_target_point.get_spec());

  if(m_display_radii)
  {
    postMessage("VIEW_CIRCLE", m_view_ext_radius.get_spec());
    postMessage("VIEW_CIRCLE", m_view_int_radius.get_spec());
  }
}


//-----------------------------------------------------------
// Procedure: updateAbsoluteTargetPoint

void Formation_Behavior::updateAbsoluteTargetPoint(bool active)
{
  m_absolute_target_point.set_vertex(m_nav_x + m_relative_target_point.x(),
                                      m_nav_y + m_relative_target_point.y(),
                                      -m_nav_depth + m_relative_target_point.z());
  m_absolute_target_point.set_active(active);

  m_view_ext_radius.setX(m_absolute_target_point.x());
  m_view_ext_radius.setY(m_absolute_target_point.y());
  m_view_ext_radius.set_active(active);

  m_view_int_radius.setX(m_absolute_target_point.x());
  m_view_int_radius.setY(m_absolute_target_point.y());
  m_view_int_radius.set_active(active);
}


//-----------------------------------------------------------
// Procedure: decreaseActiveRepositioningBehaviors

void Formation_Behavior::decreaseActiveRepositioningBehaviors()
{
  nb_active_repositioning_behaviors_ --;

  if(nb_active_repositioning_behaviors_ == 0)
  {
    drifting_date_ = MOOSTime();
    total_time_repositioning_ += MOOSTime() - repositioning_date_;
  }
}


//-----------------------------------------------------------
// Procedure: increaseActiveRepositioningBehaviors

void Formation_Behavior::increaseActiveRepositioningBehaviors()
{
  if(nb_active_repositioning_behaviors_ == 0)
  {
    repositioning_date_ = MOOSTime();
    total_time_drifting_ += MOOSTime() - drifting_date_;
  }

  nb_active_repositioning_behaviors_ ++;
}


//-----------------------------------------------------------
// Procedure: getTotalTimeRepositioning

double Formation_Behavior::getTotalTimeRepositioning()
{
  if(nb_active_repositioning_behaviors_ == 0)
    return total_time_repositioning_;

  else
    return total_time_repositioning_ + MOOSTime() - repositioning_date_;
}


//-----------------------------------------------------------
// Procedure: getTotalTimeDrifting

double Formation_Behavior::getTotalTimeDrifting()
{
  if(nb_active_repositioning_behaviors_ != 0)
    return total_time_drifting_;

  else
    return total_time_drifting_ + MOOSTime() - drifting_date_;
}


//-----------------------------------------------------------
// Procedure: updateStatistics

void Formation_Behavior::updateStatistics()
{
  double total_time_repositioning = Formation_Behavior::getTotalTimeRepositioning();
  double total_time_drifting = Formation_Behavior::getTotalTimeDrifting();
  double ratio = 100. * total_time_drifting / (total_time_drifting + total_time_repositioning);
  postMessage("TIME_DRIFTING", total_time_drifting);
  postMessage("TIME_REPOSITIONING", total_time_repositioning);
  postMessage("RATIO_RECORDING", ratio);
  postMessage("NB_ACTIVE_REPOS_BHV", nb_active_repositioning_behaviors_);

  if(m_display_statistics)
  {
    m_label_statistics.set_label(m_us_name + "_stats");
    m_label_statistics.set_active("true");
    m_label_statistics.set_vertex(m_nav_x, m_nav_y + 15);
    ostringstream strs;
    strs << round(ratio) << "%";
    m_label_statistics.set_msg(strs.str());
    m_label_statistics.set_color("label", "ghostwhite");
    postMessage("VIEW_POINT", m_label_statistics.get_spec());
  }
}


//-----------------------------------------------------------
// Procedure: filterCentroidTargetpoint

void Formation_Behavior::filterCentroidTargetpoint()
{
  m_list_previous_targetpoints.push_front(m_relative_target_point);

  if(m_list_previous_targetpoints.size() > m_filter_size_targetpoints)
    m_list_previous_targetpoints.pop_back();

  double x = 0, y = 0;

  for(list<XYPoint>::iterator previous_targetpoint = m_list_previous_targetpoints.begin() ;
      previous_targetpoint != m_list_previous_targetpoints.end() ;
      ++previous_targetpoint)
  {
    x += previous_targetpoint->x();
    y += previous_targetpoint->y();
  }

  x /= m_list_previous_targetpoints.size();
  y /= m_list_previous_targetpoints.size();

  m_relative_target_point.set_vertex(x, y);
}
