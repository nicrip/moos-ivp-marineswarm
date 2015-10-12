/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: PIDEngine.cpp                                        */
/*    DATE: Jul 31st, 2005 Sunday in Montreal                    */
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
/* Application :  pMarinePIDFreedom_CGG                          */
/*                                                               */
/*****************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <iostream>
#include <cmath>
#include "PIDEngine.h"
#include "MBUtils.h"
#include "AngleUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;

//-----------------------------------------------------------
// Procedure: Constructor

PIDEngine::PIDEngine()
{
  m_current_time = 0;
}

//------------------------------------------------------------
// Procedure: getDesiredRudder
// Heading angles are processed in degrees

double PIDEngine::getDesiredRudder(double desired_heading,
				   double current_heading,
				   double max_rudder)
{
  desired_heading = angle180(desired_heading);
  double heading_error = current_heading - desired_heading;
  heading_error = angle180(heading_error);
  double desired_rudder = 0;
  m_heading_pid.Run(degToRadians(heading_error), m_current_time, desired_rudder); // PID coef are found for heading errors in radians
    
  // Enforce limit on desired rudder
  MOOSAbsLimit(desired_rudder,max_rudder);

  string rpt = "PID_COURSE: ";
  rpt += " (Want):" + doubleToString(desired_heading);
  rpt += " (Curr):" + doubleToString(current_heading);
  rpt += " (Diff):" + doubleToString(heading_error);
  rpt += " RUDDER:" + doubleToString(desired_rudder);
  m_pid_report.push_back(rpt);
  return(desired_rudder);
}

//------------------------------------------------------------
// Procedure: getDesiredThrust - modified by Nathan

double PIDEngine::getDesiredThrust(double desired_speed, 
				   double current_speed,
				   double current_thrust,
				   double max_thrust,
           double max_thrust_force,
           double drag_coef,
				   bool use_speed_pid)
{
// We currently apply a PID controller to the problem.
// However, we cannot really use a feedback-loop for speed, since it is not measured
// current_speed should be simulated

  double desired_thrust = current_thrust;
  double speed_error  = desired_speed - current_speed;

  // IF we want to use the PID contoller
  if (use_speed_pid)
  {
    double delta_thrust = 0;
  
    m_speed_pid.Run(speed_error,  m_current_time, delta_thrust);
    desired_thrust += delta_thrust;
  }
  
  // ELSE we set the thrust_force to the corresponding axial drag
  else
  {
	  // We can use drag = Xuu * u * abs(u) or a pre-define DragMap (from the library SimMap)
      double thrust_force = drag_coef * desired_speed * fabs(desired_speed);
    
    // this thrust_force(newtons) has to be converted to desired_thrust(0-100)
      desired_thrust = thrust_force / max_thrust_force * 100;
  }
  
  if(desired_thrust < 0)
    desired_thrust = 0;

  if(use_speed_pid) {
    string rpt = "PID_SPEED: ";
    rpt += " (Want):" + doubleToString(desired_speed);
    rpt += " (Curr):" + doubleToString(current_speed);
    rpt += " (Diff):" + doubleToString(speed_error);
    rpt += " THRUST:" + doubleToString(desired_thrust);
    m_pid_report.push_back(rpt);
  }    
  else {
    string rpt = "DRAGMAP_SPEED: ";
    rpt += " (Want):" + doubleToString(desired_speed);
    rpt += " (Curr):" + doubleToString(current_speed);
    rpt += " (Diff):" + doubleToString(speed_error);
    rpt += " THRUST:" + doubleToString(desired_thrust);
    m_pid_report.push_back(rpt);
  }
  
  // Enforce limit on desired thrust
  MOOSAbsLimit(desired_thrust,max_thrust);
  return(desired_thrust);
}

//------------------------------------------------------------
// Procedure: getDesiredElevator
// Elevator angles and pitch are processed in radians

double PIDEngine::getDesiredElevator(double desired_depth,
				     double current_depth,
				     double current_pitch,
				     double max_pitch,	
				     double max_elevator)
{
  double desired_elevator = 0;
  double desired_pitch = 0;
  double depth_error = current_depth - desired_depth;
  m_z_to_pitch_pid.Run(depth_error, m_current_time, desired_pitch);

  // Enforce limits on desired pitch
  MOOSAbsLimit(desired_pitch,max_pitch);

  double pitch_error = current_pitch - desired_pitch;
  m_pitch_pid.Run(pitch_error, m_current_time, desired_elevator);

  // Enforce elevator limit
  MOOSAbsLimit(desired_elevator,max_elevator);
  
  string rpt = "PID_DEPTH: ";
  rpt += " (pitch:" + doubleToString(radToDegrees(current_pitch));
  rpt += " (Want):" + doubleToString(desired_depth);
  rpt += " (Curr):" + doubleToString(current_depth);
  rpt += " (Diff):" + doubleToString(depth_error);
  rpt += " ELEVATOR:" + doubleToString(desired_elevator);
  m_pid_report.push_back(rpt);

  return(desired_elevator);
}

//-----------------------------------------------------------
// Procedure: setPID

void PIDEngine::setPID(int ix, ScalarPID g_pid)
{
  if(ix==0) 
    m_heading_pid = g_pid;
  else if(ix==1) 
    m_speed_pid = g_pid;
  else if(ix==2) 
    m_z_to_pitch_pid = g_pid;
  else if(ix==3) 
    m_pitch_pid = g_pid;
}










