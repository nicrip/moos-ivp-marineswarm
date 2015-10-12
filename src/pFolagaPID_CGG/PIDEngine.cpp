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
/* Application :  pFolagaPID_CGG                                 */
/*                                                               */
/*****************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <cmath>
#include <iostream>
#include "MBUtils.h"
#include "AngleUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "PIDEngine.h"

using namespace std;

//-----------------------------------------------------------
// Procedure: Constructor

PIDEngine::PIDEngine()
{
  m_current_time = 0;
}


//------------------------------------------------------------
// Procedure: getDesiredSurgeThrust
// If spdPID_active is TRUE speed is controlled via PID. 
// If not, thrust is set from the DragMap

double PIDEngine::getDesiredSurgeThrust(double desired_speed, 
				   double current_speed,
				   double current_thrust,
				   double max_thrust,
           double max_speed,
           double current_heading,
           double desired_heading,
           double heading_limit,
           DragMap dmap,
           bool spdPID_active)
{
  double speed_error  = desired_speed - current_speed;
  double delta_thrust = 0;
  double desired_thrust = current_thrust;
  double heading_error = desired_heading - current_heading;
  heading_error = angle180(heading_error);
  double reduction_factor = fmax(0,(heading_limit - fabs(heading_error)) / heading_limit);

  // if spdPID_active is TRUE speed is controlled via PID
  if(spdPID_active) {
    m_speed_pid.Run(speed_error, m_current_time, delta_thrust);
    delta_thrust *= reduction_factor;
    desired_thrust += delta_thrust;
  }
  
  // ELSE thrust is set from the DragMap
  else {
    double max_thrust_force = dmap.getDragValue(max_speed); // in newtons
    desired_thrust = dmap.getDragValue(desired_speed) / max_thrust_force * 100;
    desired_thrust *= reduction_factor;
  }
  
  if(desired_thrust < 0)
    desired_thrust = 0;

  if(spdPID_active) {
    string rpt = "PID_SPEED: ";
    rpt += " (Want):" + doubleToString(desired_speed);
    rpt += " (Curr):" + doubleToString(current_speed);
    rpt += " (Diff):" + doubleToString(speed_error);
    rpt += " (Delt):" + doubleToString(delta_thrust);
    rpt += " (Thrust):" + doubleToString(desired_thrust);
    m_pid_report.push_back(rpt);
  }    
  else {
    string rpt = "PID_SPEED: ";
    rpt += " (Want):" + doubleToString(desired_speed);
    rpt += " (Curr):" + doubleToString(current_speed);
    rpt += " (Thrust):" + doubleToString(desired_thrust);
    m_pid_report.push_back(rpt);
  }
  
  // Enforce limit on desired thrust
  MOOSAbsLimit(desired_thrust,max_thrust);

  return(desired_thrust);
}


//------------------------------------------------------------
// Procedure: getDesiredHeadingThrust
// heading angles are processed in degrees

double PIDEngine::getDesiredHeadingThrust(double desired_heading,
				   double current_heading,
				   double max_heading_thrust)
{
  desired_heading = angle180(desired_heading);
  double heading_error =  desired_heading - current_heading;
  heading_error = angle180(heading_error);
  double desired_heading_thrust = 0;
  m_heading_pid.Run(heading_error, m_current_time, desired_heading_thrust);
    
  // Enforce limit on desired heading thrust
  MOOSAbsLimit(desired_heading_thrust,max_heading_thrust);

  string rpt = "PID_COURSE: ";
  rpt += " (Want):" + doubleToString(desired_heading);
  rpt += " (Curr):" + doubleToString(current_heading);
  rpt += " (Diff):" + doubleToString(heading_error);
  rpt += " RUDDER:" + doubleToString(desired_heading_thrust);
  m_pid_report.push_back(rpt);
  
  return(desired_heading_thrust);
}


//------------------------------------------------------------
// Procedure: getDesiredPitchThrust
// pitch angles are processed in degrees

double PIDEngine::getDesiredPitchThrust(double desired_depth,
				     double current_depth,
				     double current_pitch,
				     double max_pitch,
				     double max_pitch_thrust)
{
  double desired_pitch_thrust = 0.0;
  double desired_pitch = 0.0;
  double depth_error = current_depth - desired_depth;
  m_z_to_pitch_pid.Run(depth_error, m_current_time, desired_pitch);
  
  // Enforce limits on desired pitch
  MOOSAbsLimit(desired_pitch,max_pitch);

  double pitch_error = desired_pitch - current_pitch;
  m_pitch_pid.Run(pitch_error, m_current_time, desired_pitch_thrust);

  // Enforce pitch thrust limit
  MOOSAbsLimit(desired_pitch_thrust,max_pitch_thrust);
  
  string rpt = "PID_DEPTH: ";
  rpt += " (Want):" + doubleToString(desired_depth);
  rpt += " (Curr):" + doubleToString(current_depth);
  rpt += " (Diff):" + doubleToString(depth_error);
  rpt += " (pitch_thrust):" + doubleToString(desired_pitch_thrust);
  rpt += " (current_pitch):" + doubleToString(current_pitch); 
  m_pid_report.push_back(rpt);

  return(desired_pitch_thrust);
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












