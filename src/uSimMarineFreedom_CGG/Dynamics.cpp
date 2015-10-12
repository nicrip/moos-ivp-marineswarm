/* Name : Dynamics.cpp
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
#include <cmath>
#include "Dynamics.h"
#include "Coef_Remus.h"

using namespace std;

//------------------------------------------------------------------------
// Constructor

Dynamics::Dynamics()
{
  // Minv is coded directly to avoid having to inverse a matrix. 
  // It should be re-calculated and modified here every time the model of AUV is changed
  // The MATLAB script that has been used can be found in the same folder (Minv_script.m)
  
  m_Minv[0][0] = 0.0319;
  m_Minv[0][1] = 0.;
  m_Minv[0][2] = 0.0001;
  m_Minv[0][3] = 0.;
  m_Minv[0][4] = -0.0023;
  m_Minv[0][5] = 0.;
  m_Minv[1][0] = 0.;
  m_Minv[1][1] = 0.0156;
  m_Minv[1][2] = 0;
  m_Minv[1][3] = 0.0377;
  m_Minv[1][4] = 0.;
  m_Minv[1][5] = 0.0036;
  m_Minv[2][0] = 0.0001;
  m_Minv[2][1] = 0.;
  m_Minv[2][2] = 0.0153;
  m_Minv[2][3] = 0.;
  m_Minv[2][4] = -0.0035;
  m_Minv[2][5] = 0.;
  m_Minv[3][0] = 0.;
  m_Minv[3][1] = 0.0377;
  m_Minv[3][2] = 0.;
  m_Minv[3][3] = 4.1330;
  m_Minv[3][4] = 0.;
  m_Minv[3][5] = 0.0087;
  m_Minv[4][0] = -0.0023;
  m_Minv[4][1] = 0.;
  m_Minv[4][2] = -0.0035;
  m_Minv[4][3] = 0.;
  m_Minv[4][4] = 0.1210;
  m_Minv[4][5] = 0.;
  m_Minv[5][0] = 0.;
  m_Minv[5][1] = 0.0036;
  m_Minv[5][2] = 0.;
  m_Minv[5][3] = 0.0087;
  m_Minv[5][4] = 0.;
  m_Minv[5][5] = 0.1209;
}


//------------------------------------------------------------------------
// Destructor

Dynamics::~Dynamics()
{
  
}

//------------------------------------------------------------------------
// Procedure : euler

void Dynamics::euler(double& x, double xdot, double delta_time)
{
  x = x + xdot * delta_time;
}

// Runge-Kutta integration method should be considered as well for better performances


//------------------------------------------------------------------------
// Procedure : propagate

void Dynamics::propagate(double& x,
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
                         double speed_threshold)
{
  // Assign values
 
  double Xprop = thrust_force;
  double Kprop = 0; // no mapping from thrust to Kprop 
  double delta_r = rudder_angle;
  double delta_s = elevator_angle;

  double s1 = sin(phi);
  double c1 = cos(phi);
  double s2 = sin(theta);
  double c2 = cos(theta);
  double t2 = tan(theta);
  double s3 = sin(psi);
  double c3 = cos(psi);

  // Hydrostatics
  
  double XHS = -(W - B) * s2;
  double YHS =  (W - B) * c2 * s1;
  double ZHS =  (W - B) * c2 * c1;
  double KHS = -(yg * W) * c2 * c1 - (zg * W) * c2 * s1;
  double MHS = -(zg * W) * s2      - (xg * W) * c2 * c1;
  double NHS = -(xg * W) * c2 * s1 - (yg * W) * s2;

  // Total forces
  
  double X = XHS + Xuu * u * fabs(u) + (Xwq - m) * w * q + (Xqq + m * xg) * q * q + (Xvr + m) * v * r + (Xrr + m * xg) * r * r - m * yg * p * q - m * zg * p * r + Xprop;

  double Y = YHS + Yvv * v * fabs(v) + Yrr * r * fabs(r) + m * yg * r * r + (Yur - m) * u * r + (Ywp + m) * w * p + (Ypq - m * xg) * p * q + Yuv * u * v + m * yg * p * p - m * zg * q * r + Yuudr * u * u * delta_r; 

  double Z = ZHS + Zww * w * fabs(w) + Zqq * q * fabs(q) + (Zuq + m) * u * q + (Zvp - m) * v * p + (Zrp - m * xg) * r * p + Zuw * u * w + m * zg * (p * p + q * q) - m * yg * r * q + Zuuds * u * u * delta_s;

  double K = KHS + Kpp * p * fabs(p) - (Izz - Iyy) * q * r + m * yg * (u * q - v * p) - m * zg * (w * p - u * r) + Kprop;

  double M = MHS + Mww * w * fabs(w) + Mqq * q * fabs(q) + (Muq - m * xg) * u * q + (Mvp + m * xg) * v * p + (Mrp - (Ixx - Izz)) * r * p + m * zg * (v * r - w * q) + Muw * u * w + Muuds * u * u * delta_s;

  double N = NHS + Nvv * v * fabs(v) + Nrr * r * fabs(r) + (Nur - m * xg) * u * r  + (Nwp - m * xg) * w * p + /*(Npq - (Iyy - Ixx)) * p * q - m * yg * (v * r - w * q) + Nuv * u * v */+ Nuudr * u * u * delta_r;
  
  // linear drag for speed close to zero
  
  if (u < speed_threshold && u!=0)
  {
    X = XHS + (Xuu * speed_threshold * fabs(speed_threshold)) /* u / abs(u) */  * (u / speed_threshold) + (Xwq - m) * w * q + (Xqq + m * xg) * q * q + (Xvr + m) * v * r + (Xrr + m * xg) * r * r - m * yg * p * q - m * zg * p * r + Xprop;
  }
  
  // make Xdot

  double xdot     = c3*c2 * u + (c3*s2*s1-s3*c1) * v + (s3*s1+c3*c1*s2) * w;
  double ydot     = s3*c2 * u + (c1*c3+s1*s2*s3) * v + (c1*s2*s3-c3*s1) * w;
  double zdot     = -s2   * u + c2*s1            * v + c1*c2            * w;
  double phidot   =         p + s1*t2            * q + c1*t2            * r;
  double thetadot =             c1               * q - s1               * r;
  double psidot   =             s1/c2            * q + c1/c2            * r;
  
  double udot = m_Minv[0][0] * X + m_Minv[0][1] * Y + m_Minv[0][2] * Z + m_Minv[0][3] * K + m_Minv[0][4] * M + m_Minv[0][5] * N;
  double vdot = m_Minv[1][0] * X + m_Minv[1][1] * Y + m_Minv[1][2] * Z + m_Minv[1][3] * K + m_Minv[1][4] * M + m_Minv[1][5] * N;
  double wdot = m_Minv[2][0] * X + m_Minv[2][1] * Y + m_Minv[2][2] * Z + m_Minv[2][3] * K + m_Minv[2][4] * M + m_Minv[2][5] * N;
  double pdot = m_Minv[3][0] * X + m_Minv[3][1] * Y + m_Minv[3][2] * Z + m_Minv[3][3] * K + m_Minv[3][4] * M + m_Minv[3][5] * N;
  double qdot = m_Minv[4][0] * X + m_Minv[4][1] * Y + m_Minv[4][2] * Z + m_Minv[4][3] * K + m_Minv[4][4] * M + m_Minv[4][5] * N;
  double rdot = m_Minv[5][0] * X + m_Minv[5][1] * Y + m_Minv[5][2] * Z + m_Minv[5][3] * K + m_Minv[5][4] * M + m_Minv[5][5] * N;


  // propagate X with Euler Method
  
  euler(x, xdot, delta_time);
  euler(y, ydot, delta_time);
  euler(z, zdot, delta_time);
  euler(phi,   phidot,   delta_time);
  euler(theta, thetadot, delta_time);
  euler(psi,   psidot,   delta_time);
  
  euler(u, udot, delta_time);
  euler(v, vdot, delta_time);
  euler(w, wdot, delta_time);
  euler(p, pdot, delta_time);
  euler(q, qdot, delta_time);
  euler(r, rdot, delta_time);

}



