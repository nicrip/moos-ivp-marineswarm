
#include <iostream>
#include "ThrustMap.h"
#include "MBUtils.h"

using namespace std;

//----------------------------------------------------------------
// Constructor

ThrustMap::ThrustMap()
{
  m_reflect_negative = false;

  m_figlog.setLabel("ThrustMap");

  m_factor = 0;
}

//----------------------------------------------------------------
// Procedure: setThrustFactor

void ThrustMap::setThrustFactor(double value)
{
  setFactor(value);
}

//----------------------------------------------------------------
// Procedure: getThrustValue

double ThrustMap::getThrustValue(double speed) const
{
  return(getYValue(speed));
}

//----------------------------------------------------------------
// Procedure: getSpeedValue

double ThrustMap::getSpeedValue(double thrust) const
{
  return(getXValue(thrust));
}

//----------------------------------------------------------------
// Procedure: getThrustFactor

double ThrustMap::getThrustFactor() const
{
  return(getFactor());
}

//----------------------------------------------------------------
// Procedure: usingThrustFactor

bool   ThrustMap::usingThrustFactor() const
{
  return(usingFactor());
}

