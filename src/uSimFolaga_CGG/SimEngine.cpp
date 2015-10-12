/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: SimEngine.cpp                                        */
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
#include "AngleUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "SimEngine.h"

using namespace std;


//--------------------------------------------------------------------
// Procedure: propagatePosition

void SimEngine::propagatePosition(NodeRecord &record, 
                                  double delta_time,
                                  double prior_heading,
                                  double prior_speed,
                                  double drift_x,
                                  double drift_y)
{
  double speed = (record.getSpeed() + prior_speed) / 2;

  double s = sin(degToRadians(prior_heading)) +
    sin(degToRadians(record.getHeading()));

  double c = cos(degToRadians(prior_heading)) +
    cos(degToRadians(record.getHeading()));

  double hdg_rad = atan2(s, c);

  double prev_x = record.getX();
  double prev_y = record.getY();

  double cos_ang = cos(hdg_rad);
  double sin_ang = sin(hdg_rad);

  double horizontal_speed = cos(degToRadians(record.getPitch())) * speed;

  double xdot  = (sin_ang * horizontal_speed);
  double ydot  = (cos_ang * horizontal_speed);

  double new_x = prev_x + (xdot * delta_time) + (drift_x * delta_time);
  double new_y = prev_y + (ydot * delta_time) + (drift_y * delta_time);
  double new_time = record.getTimeStamp() + delta_time;
  double new_sog = hypot((xdot + drift_x), (ydot + drift_y));
  double new_hog = relAng(prev_x, prev_y,  new_x, new_y);

  record.setX(new_x);
  record.setY(new_y);
  record.setTimeStamp(new_time);
  record.setSpeedOG(new_sog);
  record.setHeadingOG(new_hog);
}


//--------------------------------------------------------------------
// Procedure: propagateDepth

void SimEngine::propagateDepth(NodeRecord& record,
                               double delta_time,
                               double drift_z)
             
{
  double speed = record.getSpeed();
  double prev_depth = record.getDepth();
  double new_depth = prev_depth;
  
  if(speed > 0.0) {
    // Neutral buoyancy
    // Change of depth is only due to pitch
    // getPitch() gives pitch in degrees
    
    double pitch = record.getPitch();
    double vertical_speed = speed * sin(degToRadians(pitch)); // directed upward
    new_depth += -vertical_speed * delta_time;
  }
  
  // drift_z is given positive upward
  new_depth += - drift_z * delta_time;
  
  // Ensure positive depth
  if (new_depth < 0)
    new_depth = 0.0;
    
  record.setDepth(new_depth);
}


//--------------------------------------------------------------------
// Procedure: propagateSpeed

void SimEngine::propagateSpeed(NodeRecord& record, 
                                double delta_time, 
                                double surge_thrust,
                                double mass,
                                double max_speed,
                                DragMap drag_map)
{
  if(delta_time <= 0)
    return;

  double prev_speed  = record.getSpeed();
  
  // thrust_force (Newtons) as a fraction of the drag at max_speed
  // surge_thrust is between 0 and 100
  double thrust_force = surge_thrust / 100 * drag_map.getDragValue(max_speed);
  
  // current drag
  double drag = drag_map.getDragValue(prev_speed);
    
  // propagate speed
  double speeddot = 1 / mass * (thrust_force - drag);
  
  // Euler method
  double next_speed = prev_speed + speeddot * delta_time; 
  
  record.setSpeed(next_speed);
}


//--------------------------------------------------------------------
// Procedure: propagateHeading

void SimEngine::propagateHeading(NodeRecord& record, 
                                  double delta_time,
                                  // fot thrust action
                                  double heading_thrust,
                                  double max_rotate_speed,
                                  // for heading noise
                                  double& heading_rate_noise,
                                  double sigma,
                                  double mu,
                                  double saturation)
{
  // Thrust action
  double headingdot = heading_thrust / 100 * max_rotate_speed;
  
  // Gaussian Markov process for heading rate noise (the time-derivative of the heading angle)
    double white_noise = MOOSWhiteNoise(sigma);
    heading_rate_noise += delta_time * (- mu * heading_rate_noise + white_noise);
    
    if (heading_rate_noise > fabs(saturation))
      heading_rate_noise = fabs(saturation);
    if (heading_rate_noise < -fabs(saturation))
      heading_rate_noise = -fabs(saturation);
    
    headingdot += heading_rate_noise;
  
  // Propagate heading
  double prev_heading = record.getHeading();
  double new_heading  = angle360(prev_heading + headingdot * delta_time); // Euler method
  
  record.setHeading(new_heading);
}


//--------------------------------------------------------------------
// Procedure: propagatePitch

void SimEngine::propagatePitch(NodeRecord& record, 
                              double delta_time, 
                              double pitch_thrust,
                              double max_rotate_speed)
{
  double pitchdot = pitch_thrust / 100 * max_rotate_speed;
  
  double prev_pitch = record.getPitch();
  double new_pitch  = angle180(prev_pitch + pitchdot * delta_time); // Euler method
  
  record.setPitch(new_pitch);
}




