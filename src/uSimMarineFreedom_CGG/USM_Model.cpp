/* Name : USM_Model.cpp
 * Author : Nathan Vandervaeren
 * Date : April 2014
 * 
 * This class is instantiated in USM_MOOSApp_6DOF to handle the
 * propagation of the AUV model.
 */
 
 
#include <iostream>
#include <cmath> 
#include <cstdlib>
#include "USM_Model.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "Node/NodeRecord6DOF.h"


using namespace std;

//------------------------------------------------------------------------
// Constructor

USM_Model::USM_Model() 
{
  // Initalize the configuration variables
  m_paused       = false;

  // Initalize the state variables
  m_thrust_force    = 0;  // radians
  m_rudder_angle    = 0;  // radians
  m_elevator_angle  = 0;  // radians
  m_drift_x         = 0;  // m/s
  m_drift_y         = 0;  // m/s
  m_drift_z         = 0;  // m/s
  m_speed_threshold = 0.1;// m/s
  m_water_depth     = 0;  // zero means nothing known, no altitude reported

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
  param = stripBlankEnds(toupper(param));
  if(param == "START_X") {
    m_record.setY(value); // NodeRecord6DOF is expressed in NED coordinates
  }
  else if(param == "START_Y") {
    m_record.setX(value); // NodeRecord6DOF is expressed in NED coordinates
  }  
  else if(param == "START_HEADING") {
    m_record.setHeading(value); // heading is in degrees while Psi (yaw angle) is in radians
    m_record.setPsi(degToRadians(value)); // NodeRecord6DOF is expressed in NED coordinates
  }
  else if(param == "START_SPEED") {
    m_record.setSurge(value);
    m_record.setSpeed(value);
  }
  else if(param == "START_DEPTH") {
    m_record.setDepth(value);
    m_record.setZ(value); // NodeRecord6DOF is expressed in NED coordinates
    if(value < 0) {
      m_record.setDepth(0);
      m_record.setZ(0);
      return(false);
    }
  }
  else if(param == "DRIFT_X") 
    m_drift_y = value; // NodeRecord6DOF is expressed in NED coordinates
  else if(param == "DRIFT_Y") 
    m_drift_x = value; // NodeRecord6DOF is expressed in NED coordinates
  else if(param == "DRIFT_Z") 
    m_drift_z = -value; // NodeRecord6DOF is expressed in NED coordinates
  else if(param == "SPEED_THRESHOLD") 
    m_speed_threshold = value;
  else if(param == "WATER_DEPTH") {
    if(value >= 0)
      m_water_depth = value;
    else
      return(false);
  }
  else
    return(false);
  return(true);
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

  propagateNodeRecord6DOF(m_record, delta_time);

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


//---------------------------------------------------------------------
// Procedure: getDriftSummary

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
      m_record.setY(dval); // NodeRecord6DOF is expressed in NED coordinates
    }
    else if(param == "y") {
      m_record.setX(dval); // NodeRecord6DOF is expressed in NED coordinates
    }
    else if((param == "heading") || (param=="deg") || (param=="hdg")) {
      m_record.setHeading(dval);           // heading is in degrees while Psi (yaw angle) is in radians
      m_record.setPsi(degToRadians(dval)); // NodeRecord6DOF is expressed in NED coordinates
    }
    else if((param == "speed") || (param == "spd")) {
      m_record.setSurge(dval);
      m_record.setSpeed(dval);
    }
    else if((param == "depth") || (param == "dep")) {
      m_record.setDepth(dval);
      m_record.setZ(dval); // NodeRecord6DOF is expressed in NED coordinates
    }
    else
      return(false);
  }
  return(true);
}

//------------------------------------------------------------------------
// Procedure: propagateNodeRecord6DOF

void USM_Model::propagateNodeRecord6DOF(NodeRecord6DOF& record, 
				    double delta_time)
{
  m_simengine.propagate(record, m_thrust_force, m_rudder_angle, m_elevator_angle, delta_time, m_drift_x, m_drift_y, m_drift_z, m_speed_threshold);

  // If m_water_depth > 0 then something is known about the present
  // water depth and thus we update the vehicle altitude.
  if(m_water_depth > 0)
  {
    double depth = record.getDepth();
    double altitude = m_water_depth - depth;
    if(altitude < 0) 
      altitude = 0;
    record.setAltitude(altitude);
  }
}


