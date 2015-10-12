/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: VortexCurrent.cpp                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <iostream>
#include <cmath>
#include <cstdlib> // rand()
#include "AngleUtils.h"
#include "VortexCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

VortexCurrent::VortexCurrent()
{
  m_position_x = 0.;
  m_position_y = 0.;
  m_magnitude = 1.;
  m_radius = 1000.;
}


//---------------------------------------------------------
// Procedure: Destructor

VortexCurrent::~VortexCurrent()
{

}


//---------------------------------------------------------
// Procedure: setParam

void VortexCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
    
  if(param == "VORTEX__POS_X")
    m_position_x = atof(value.c_str());
    
  if(param == "VORTEX__POS_Y")
    m_position_y = atof(value.c_str());
    
  if(param == "VORTEX__MAGNITUDE")
    m_magnitude = atof(value.c_str());
    
  if(param == "VORTEX__RADIUS")
    m_radius = atof(value.c_str());
}


//---------------------------------------------------------
// Procedure: init

bool VortexCurrent::init()
{
  return true;
}


//---------------------------------------------------------
// Procedure: getType

std::string VortexCurrent::getType()
{
  return "VORTEX";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool VortexCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  double dx = x - m_position_x;
  double dy = y - m_position_y;
  double alpha = (M_PI / 2.) - atan2(dy, dx);
  double r = hypot(dx,dy);
  
  double current = m_magnitude * max((r * (m_radius - r)) / (m_radius * m_radius), 0.02);
  
  drift_x = cos(alpha) * current;
  drift_y = sin(-alpha) * current;
  drift_z = 0.;
  
  return true;
}
