/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: MultiSpectralCurrent.cpp                             */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <cmath>
#include <iostream>
#include <cstdlib> // rand()
#include "MultiSpectralCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

MultiSpectralCurrent::MultiSpectralCurrent()
{
  m_coriolis_parameter = 0.000126; // 1.26e-4
  m_gravity_constant = 9.81;
}


//---------------------------------------------------------
// Procedure: Destructor

MultiSpectralCurrent::~MultiSpectralCurrent()
{

}


//---------------------------------------------------------
// Procedure: setParam

void MultiSpectralCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
    
  if(param == "SPECTRAL__CORIOLIS_PARAMETER")
    setCoriolisParameter(atof(value.c_str()));
    
  if(param == "SPECTRAL__GRAVITY_CONSTANT")
    setGravityConstant(atof(value.c_str()));
    
  if(param == "ADD_SPECTRAL_CURRENT")
  {
    string sub_values[2] = { "", "" };
    
    int k = 0;
    for(int i = 0 ; i < value.length() ; i++)
    {
      if(value[i] == ',')
      {
        k = 1;
        continue;
      }
      
      sub_values[k] += value[i];
    }
    
    double wavenumber = atof(sub_values[0].c_str());
    double SSH_amplitude = atof(sub_values[1].c_str());
    m_vector_spectral_currents.push_back(new SpectralCurrent(wavenumber, SSH_amplitude));
    m_vector_spectral_currents.back()->init();
  }
}


//---------------------------------------------------------
// Procedure: init

bool MultiSpectralCurrent::init()
{
  return true;
}


//---------------------------------------------------------
// Procedure: setCoriolisParameter

void MultiSpectralCurrent::setCoriolisParameter(double value)
{
  m_coriolis_parameter = value;
}


//---------------------------------------------------------
// Procedure: GravityConstant

void MultiSpectralCurrent::setGravityConstant(double value)
{
  m_gravity_constant = value;
}


//---------------------------------------------------------
// Procedure: type

std::string MultiSpectralCurrent::getType()
{
  return "MULTI_SPECTRAL";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool MultiSpectralCurrent::getCurrent(double x, double y, double depth,
                                      double time,
                                      double& drift_x,
                                      double& drift_y,
                                      double& drift_z)
{
  drift_x = 0.;
  drift_y = 0.;
  drift_z = 0.;
  
  double this_spectral_drift_x, this_spectral_drift_y, this_spectral_drift_z;
  
  for(vector<SpectralCurrent*>::iterator spectral_current = m_vector_spectral_currents.begin() ; 
    spectral_current != m_vector_spectral_currents.end() ; 
    spectral_current++)
  {
    if(!(*spectral_current)->getCurrent(x, y, depth, time, 
                                    this_spectral_drift_x,
                                    this_spectral_drift_y,
                                    this_spectral_drift_z))
    {
      return false;
    }
    
    drift_x += this_spectral_drift_x;
    drift_y += this_spectral_drift_y;
    drift_z += this_spectral_drift_z;
  }
  
  return true;
}
