/* Name : SimEngine.cpp
 * Author : Nathan Vandervaeren
 * Date : April 2014
 * 
 * This class is used to apply AUV dynamics on a NodeRecord6DOF,
 * integrating drift_x, drift_y and drift_z.
 * 
 * It also calculate additional variables such as speed, 
 * heading in degrees, depth, speed and heading over ground.
 * 
 * IMPORTANT : coordinates are expressed in North-East-Down(NED) coordinates.
 */ 

#include <iostream>
#include <cmath>
#include <vector>
#include "AngleUtils.h"
#include "SimEngine.h"

using namespace std;

//--------------------------------------------------------------------
// Procedure: Constructor

SimEngine::SimEngine()
{
  
}

//--------------------------------------------------------------------
// Procedure: Destructor

SimEngine::~SimEngine()
{
  
}

//--------------------------------------------------------------------
// Procedure: NEDtoENU

void SimEngine::NEDtoENU(double& x, double& y, double& z, double& phi, double& theta, double& psi)
{
  double old_x = x;
  x = y;
  y = old_x;
  z = -z;
  double old_phi = phi;
  phi = theta - M_PI_2;
  theta = old_phi + M_PI_2;
  psi = M_PI_2 - psi;
}


//--------------------------------------------------------------------
// Procedure: ENUtoNED (surprisingly, same function as NEDtoENU)

void SimEngine::ENUtoNED(double& x, double& y, double& z, double& phi, double& theta, double& psi)
{
  double old_x = x;
  x = y;
  y = old_x;
  z = -z;
  double old_phi = phi;
  phi = theta - M_PI_2;
  theta = old_phi + M_PI_2;
  psi = M_PI_2 - psi;
}


//--------------------------------------------------------------------
// Procedure: propagate

void SimEngine::propagate(NodeRecord6DOF& record, 
                          double thrust_force,
                          double rudder_angle,
                          double elevator_angle,
                          double delta_time,
                          double drift_x,
                          double drift_y,
                          double drift_z,
                          double speed_threshold)
{
  // coordinates in record are in East-North-Up, while Dynamics uses North-East-Down
  double x      = record.getX();
  double y      = record.getY();
  double z      = record.getZ();
  double phi    = record.getPhi();
  double theta  = record.getTheta();
  double psi    = record.getPsi();
  double u      = record.getSurge();
  double v      = record.getSway();
  double w      = record.getHeave();
  double p      = record.getRoll();
  double q      = record.getPitch();
  double r      = record.getYaw();
  
  double prev_x = x;
  double prev_y = y;
  
  m_dynamics.propagate(x,y,z,phi,theta,psi,u,v,w,p,q,r,thrust_force,rudder_angle,elevator_angle,delta_time,speed_threshold);
  
  // we add the drift effect
  
  x = x + drift_x * delta_time;
  y = y + drift_y * delta_time;
  z = z + drift_z * delta_time;
  
  // we calculate additional variables

  double speed    = u;   // surge only
  double heading  = angle360(radToDegrees(psi));

  double depth    = z;
  double speed_og   = hypot(x-prev_x, y-prev_y) / delta_time;
  double heading_og = relAng(prev_y, prev_x, y, x); // since relAng operates in ENU coordinates, we switch x- and y- coordinates
  
  double time_stamp = record.getTimeStamp() + delta_time;
  
  // Update propagated values in NodeReport6DOF
  
  record.setX(x);
  record.setY(y);
  record.setZ(z);
  record.setPhi(phi);
  record.setTheta(theta);
  record.setPsi(psi);
  record.setSurge(u);
  record.setSway(v);
  record.setHeave(w);
  record.setRoll(p);
  record.setPitch(q);
  record.setYaw(r);
  
  record.setSpeed(speed);
  record.setHeading(heading);
  record.setDepth(depth);
  record.setSpeedOG(speed_og);
  record.setHeadingOG(heading_og);
  record.setTimeStamp(time_stamp);
}

  


