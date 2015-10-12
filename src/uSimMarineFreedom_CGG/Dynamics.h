/* Name : Dynamics.h
 * Author : Nathan Vandervaeren
 * Date : April 2014
 * 
 * Model AUV dynamics with 6 degrees-of-freedom.
 * Vehicle coefficients are read in Coef_Remus.h
 * 
 * Equations come from T.Prestero, "Verification of a six-degree of 
 * freedom simulation model for the Remus autonomous underwater vehicle".
 * 
 * IMPORTANT : equations are expressed in North-East-Down(NED) coordinates.
 */

#include <iostream>

class Dynamics
{
public:
  Dynamics();
  ~Dynamics();
  
  void propagate(double& x,
                 double& y,
                 double& z,
                 double& phi,
                 double& theta,
                 double& psi,
                 double& u,
                 double& v,
                 double& w,
                 double& p,
                 double& q,
                 double& r,
                 double thrust_force,
                 double rudder_angle,
                 double elevator_angle,
                 double delta_time,
                 double speed_threshold);
                           
protected: // used in propagate
  void euler(double& x, double xdot, double delta_time);
  
protected:
  double m_Minv[6][6];
};


  
  
  
  
  
  
  
  
