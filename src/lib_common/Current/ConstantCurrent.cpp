/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: ConstantCurrent.cpp                                  */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <cstdlib>
#include "ConstantCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

ConstantCurrent::ConstantCurrent()
{
  m_drift_x = 0.;
  m_drift_y = 0.;
  m_drift_z = 0.;
}


//---------------------------------------------------------
// Procedure: Destructor

ConstantCurrent::~ConstantCurrent()
{
  
}


//---------------------------------------------------------
// Procedure: setParam

void ConstantCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "CONSTANT__DRIFT_X")
    setDriftX(atof(value.c_str()));
    
  if(param == "CONSTANT__DRIFT_Y")
    setDriftY(atof(value.c_str()));
    
  if(param == "CONSTANT__DRIFT_Z")
    setDriftZ(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool ConstantCurrent::init()
{
  return true;
}


//---------------------------------------------------------
// Procedure: type

std::string ConstantCurrent::getType()
{
  return "CONSTANT";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool ConstantCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  drift_x = m_drift_x;
  drift_y = m_drift_y;
  drift_z = m_drift_z;
  return true;
}


//---------------------------------------------------------
// Procedure: setDriftX

void ConstantCurrent::setDriftX(double drift)
{
  m_drift_x = drift;
}


//---------------------------------------------------------
// Procedure: setDriftY

void ConstantCurrent::setDriftY(double drift)
{
  m_drift_y = drift;
}


//---------------------------------------------------------
// Procedure: setDriftZ

void ConstantCurrent::setDriftZ(double drift)
{
  m_drift_z = drift;
}
