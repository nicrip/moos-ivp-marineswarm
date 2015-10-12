/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: ShearCurrent.cpp                                     */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <cstdlib>
#include <math.h>
#include "AngleUtils.h"
#include "ShearCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

ShearCurrent::ShearCurrent()
{
  m_gap = 0.;
  m_theta = 0.;
}


//---------------------------------------------------------
// Procedure: Destructor

ShearCurrent::~ShearCurrent()
{
  
}


//---------------------------------------------------------
// Procedure: setParam

void ShearCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "SHEAR__GAP")
    setGap(atof(value.c_str()));
    
  if(param == "SHEAR__THETA")
    setTheta(atof(value.c_str()));
    
  if(param == "SHEAR__MAX_SPEED")
    setMaxCurrentSpeed(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool ShearCurrent::init()
{
  if(m_gap <= 0)
    return false;
  
  return true;
}


//---------------------------------------------------------
// Procedure: type

std::string ShearCurrent::getType()
{
  return "SHEAR";
}


//---------------------------------------------------------
// Procedure: setGap

void ShearCurrent::setGap(double gap)
{
  m_gap = gap;
}


//---------------------------------------------------------
// Procedure: setTheta

void ShearCurrent::setTheta(double theta)
{
  m_theta = degToRadians(theta);
}


//---------------------------------------------------------
// Procedure: setMaxCurrentSpeed

void ShearCurrent::setMaxCurrentSpeed(double speed)
{
  m_max_current_speed = speed;
}


//---------------------------------------------------------
// Procedure: getCurrent

bool ShearCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  double x_gap = x * cos(m_theta) - y * sin(m_theta);
  int id_gap = x_gap / m_gap;
  
  double sign;
  if(id_gap % 2 == 0)
    sign = -1.;
  
  else
    sign = 1.;
    
  if(x_gap <= 0)
    sign = -sign;
  
  drift_y = sign * m_max_current_speed * cos(m_theta);
  drift_x = sign * m_max_current_speed * sin(m_theta);
  drift_z = 0.;
  
  return true;
}
