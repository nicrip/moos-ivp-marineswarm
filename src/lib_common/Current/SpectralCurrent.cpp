/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SpectralCurrent.cpp                                  */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <iostream>
#include <cmath>
#include <cstdlib> // rand()
#include "AngleUtils.h"
#include "SpectralCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

SpectralCurrent::SpectralCurrent()
{
  m_wavenumber = 0.;
  m_SSH_amplitude = 0.;
}


//---------------------------------------------------------
// Procedure: Constructor

SpectralCurrent::SpectralCurrent(double wavenumber, double SSH_amplitude)
{
  m_wavenumber = wavenumber;
  m_SSH_amplitude = SSH_amplitude;
}


//---------------------------------------------------------
// Procedure: Destructor

SpectralCurrent::~SpectralCurrent()
{

}


//---------------------------------------------------------
// Procedure: setParam

void SpectralCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
    
  if(param == "SPECTRAL__WAVENUMBER")
    setWavenumber(atof(value.c_str()));
    
  if(param == "SPECTRAL__SSH_AMPLITUDE")
    setSSHAmplitude(atof(value.c_str()));
    
  if(param == "SPECTRAL__CORIOLIS_PARAMETER")
    setCoriolisParameter(atof(value.c_str()));
    
  if(param == "SPECTRAL__GRAVITY_CONSTANT")
    setGravityConstant(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool SpectralCurrent::init()
{
  m_coriolis_parameter = 0.000126; // 1.26e-4
  m_gravity_constant = 9.81;
  m_phase_x = ((double)(rand()) / RAND_MAX) * 2 * M_PI;
  m_phase_y = ((double)(rand()) / RAND_MAX) * 2 * M_PI;
  m_angle = ((double)(rand()) / RAND_MAX) * 2 * M_PI;
  return true;
}


//---------------------------------------------------------
// Procedure: setWavelength

void SpectralCurrent::setWavenumber(double value)
{
  m_wavenumber = value;
}


//---------------------------------------------------------
// Procedure: setSSHAmplitude

void SpectralCurrent::setSSHAmplitude(double value)
{
  m_SSH_amplitude = value;
}


//---------------------------------------------------------
// Procedure: setCoriolisParameter

void SpectralCurrent::setCoriolisParameter(double value)
{
  m_coriolis_parameter = value;
}


//---------------------------------------------------------
// Procedure: GravityConstant

void SpectralCurrent::setGravityConstant(double value)
{
  m_gravity_constant = value;
}


//---------------------------------------------------------
// Procedure: type

std::string SpectralCurrent::getType()
{
  return "SPECTRAL";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool SpectralCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  double X =  cos(m_angle) * x + sin(m_angle) * y;
  double Y = -sin(m_angle) * x + cos(m_angle) * y;
  
  double true_k = 2 * M_PI * m_wavenumber / 1000;
  double kx = true_k / sqrt(2);
  double ky = kx;
  
  double drift_X = -m_gravity_constant / m_coriolis_parameter * (m_SSH_amplitude/100) / sqrt(2) * ky * cos(ky * Y + m_phase_y);
  double drift_Y = m_gravity_constant / m_coriolis_parameter * (m_SSH_amplitude/100) / sqrt(2) * kx * cos(kx * X + m_phase_x);
  
  drift_x = cos(-m_angle) * drift_X + sin(-m_angle) * drift_Y;
  drift_y = -sin(-m_angle) * drift_X + cos(-m_angle) * drift_Y;
  drift_z = 0.0;
  
  return true;
}
