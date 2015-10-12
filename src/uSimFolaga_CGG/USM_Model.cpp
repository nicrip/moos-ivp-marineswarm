/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USM_Model.cpp                                        */
/*    DATE: Nov 19th 2006 (as separate class under MVC paradigm  */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/
/*                                                               */
/* Modified by :  Nathan Vandervaeren                            */
/* Date :         April 2014                                     */
/* Application :  uSimFolaga_CGG                                 */
/*                                                               */
/*****************************************************************/

#include <iostream>
#include <cmath> 
#include <cstdlib>
#include "USM_Model.h"
#include "SimEngine.h"
#include "MBUtils.h"
#include "AngleUtils.h"

using namespace std;

//------------------------------------------------------------------------
// Constructor

USM_Model::USM_Model() 
{
  // Initalize the configuration variables
  m_paused               = false;

  // Initalize the state variables
  m_surge_thrust     = 0.;
  m_heading_thrust   = 0.;
  m_pitch_thrust     = 0.;
  
  m_mass                = 32.; // Folaga without seismic payload (kg)
  m_max_speed           = 1.;  // 1 m/s ~ 2 knots
  m_max_rotate_speed    = 18.; // degrees per sec
  m_heading_rate_noise  = 0.;  // degrees per sec (initially)

  m_drift_x      = 0.;
  m_drift_y      = 0.; 
  m_drift_z      = 0.;

  m_water_depth  = 0.;    // zero means nothing known, no altitude reported
  
  // for heading noise 
  m_heading_sigma       = 0.1;
  m_heading_mu          = 0.005;
  m_heading_saturation  = 1.0;   // degrees per sec
}


//------------------------------------------------------------------------
// Procedure: resetTime()

void USM_Model::resetTime(double g_curr_time)
{
  m_record.setTimeStamp(g_curr_time);
}


//------------------------------------------------------------------------
// Procedure: setParam

bool USM_Model::setParam(string param, double value)
{
  param = stripBlankEnds(tolower(param));
  if(param == "start_x") {
    m_record.setX(value);
  }
  else if(param == "start_y") {
    m_record.setY(value);
  }
  else if(param == "start_heading") {
    m_record.setHeading(value);
  }
  else if(param == "start_speed") {
    m_record.setSpeed(value);
  }
  else if(param == "start_depth") {
    m_record.setDepth(value);
    if(value < 0) {
      m_record.setDepth(0);
      return(false);
    }
  }
  else if(param == "drift_x")
    m_drift_x = value;
  else if(param == "drift_y")
    m_drift_y = value;
  else if(param == "drift_z")
    m_drift_z = value;
  else if(param == "water_depth") {
    if(value >= 0)
      m_water_depth = value;
    else
      return(false);
  }
  else if(param == "mass")
    m_mass = value;
  else if(param == "max_speed")
    m_max_speed = value;
  else if(param == "max_rotate_speed")
    m_max_rotate_speed = value;  
    
  // for heading noise
  else if(param == "heading_sigma")
    m_heading_sigma = value;  
  else if(param == "heading_mu")
    m_heading_mu = value;  
  else if(param == "heading_saturation")
    m_heading_saturation = value;  
  else
    return(false);
  return(true);
}


//------------------------------------------------------------------------
// Procedure: handleDragMap

bool USM_Model::handleDragMap(std::string value)
{
  return m_drag_map.setPoints(value);
}


//------------------------------------------------------------------------
// Procedure: propagate
//      Note: 

bool USM_Model::propagate(double g_curr_time)
{
  if(m_paused) {
    cout << "Simulator PAUSED..................." << endl;
    return(true);
  }
    
  // Calculate actual current time considering time spent paused.
  double a_curr_time = g_curr_time - m_pause_timer.get_wall_time();
  double delta_time  = a_curr_time - m_record.getTimeStamp();

  propagateNodeRecord(m_record, delta_time);
    
  return(true);
}


//------------------------------------------------------------------------
// Procedure: setPaused
//      Note: 

void USM_Model::setPaused(bool g_paused)
{
  if(m_paused == g_paused)
    return;
  else
    m_paused = g_paused;
 
  if(m_paused)
    m_pause_timer.start();
  else
    m_pause_timer.stop();  
}


//---------------------------------------------------------------------
// Procedure: getDriftSummary()

string USM_Model::getDriftSummary()
{
  double magnitude = getDriftMag();
  double angle = getDriftAng();

  string val = "ang=";
  val += doubleToStringX(angle,2);
  val += ", mag=";
  val += doubleToStringX(magnitude,2);
  val += ", xmag=";
  val += doubleToStringX(m_drift_x,3);
  val += ", ymag=";
  val += doubleToStringX(m_drift_y,3);
  val += ", zmag=";
  val += doubleToStringX(m_drift_z,3);
  return(val);
}


//------------------------------------------------------------------------
// Procedure: getDriftMag

double USM_Model::getDriftMag() const
{
  return(hypot(m_drift_x, m_drift_y));
}


//------------------------------------------------------------------------
// Procedure: getDriftAng

double USM_Model::getDriftAng() const
{
  return(relAng(0, 0, m_drift_x, m_drift_y));
}


//------------------------------------------------------------------------
// Procedure: initPosition
//
//  "x=20, y=-35, speed=2.2, heading=180, depth=20"


bool USM_Model::initPosition(const string& str)
{
  vector<string> svector = parseString(str, ',');
  unsigned int i, vsize = svector.size();
  for(i=0; i<vsize; i++) {
    svector[i] = tolower(stripBlankEnds(svector[i]));
    string param = biteStringX(svector[i], '=');
    string value = svector[i];

    // Support older style spec "5,10,180,2.0,0" - x,y,hdg,spd,dep
    if(value == "") {
      value = param;
      if(i==0)      param = "x";
      else if(i==1) param = "y";
      else if(i==2) param = "heading";
      else if(i==3) param = "speed";
      else if(i==4) param = "depth";
    }

    double dval  = atof(value.c_str());
    if(param == "x") {
      m_record.setX(dval);
    }
    else if(param == "y") {
      m_record.setY(dval);
    }
    else if((param == "heading") || (param=="deg") || (param=="hdg")) {
      m_record.setHeading(dval);
    }
    else if((param == "speed") || (param == "spd")) {
      m_record.setSpeed(dval);
    }
    else if((param == "depth") || (param == "dep")) {
      m_record.setDepth(dval);
    }
    else
      return(false);
  }
  return(true);
}


//------------------------------------------------------------------------
// Procedure: propagateNodeRecord

void USM_Model::propagateNodeRecord(NodeRecord& record, 
				    double delta_time)
{
  double prior_spd = record.getSpeed();
  double prior_hdg = record.getHeading();

  // speed - pitch - heading
  m_sim_engine.propagateSpeed(record, delta_time,
			      m_surge_thrust,
            m_mass,
            m_max_speed,
            m_drag_map);
            
  m_sim_engine.propagatePitch(record, delta_time, 
            m_pitch_thrust, 
            m_max_rotate_speed);
        
  m_sim_engine.propagateHeading(record, delta_time,
            m_heading_thrust,
            m_max_rotate_speed,
            m_heading_rate_noise,
            m_heading_sigma,
            m_heading_mu,
            m_heading_saturation);
      
  // position - depth
  m_sim_engine.propagateDepth(record, delta_time, 
            m_drift_z);
        
  m_sim_engine.propagatePosition(record, delta_time,
            prior_hdg,
            prior_spd,
            m_drift_x,
            m_drift_y);
  
  // If m_water_depth > 0 then something is known about the present
  // water depth and thus we update the vehicle altitude.
  if(m_water_depth > 0) {
    double depth = record.getDepth();
    double altitude = m_water_depth - depth;
    if(altitude < 0) 
      altitude = 0;
    record.setAltitude(altitude);
  }
}

