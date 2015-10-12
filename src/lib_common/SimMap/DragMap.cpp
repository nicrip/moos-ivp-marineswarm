
#include <iostream>
#include "DragMap.h"
#include "MBUtils.h"

using namespace std;

//----------------------------------------------------------------
// Constructor

DragMap::DragMap()
{
  m_reflect_negative = false;

  m_figlog.setLabel("DragMap");

  m_factor = 0;
}

//----------------------------------------------------------------
// Procedure: setDragFactor

void DragMap::setDragFactor(double value)
{
  setFactor(value);
}

//----------------------------------------------------------------
// Procedure: getDragValue

double DragMap::getDragValue(double speed) const
{
  return(getYValue(speed));
}

//----------------------------------------------------------------
// Procedure: getSpeedValue

double DragMap::getSpeedValue(double drag) const
{
  return(getXValue(drag));
}

//----------------------------------------------------------------
// Procedure: getDragFactor

double DragMap::getDragFactor() const
{
  return(getFactor());
}

//----------------------------------------------------------------
// Procedure: usingDragFactor

bool   DragMap::usingDragFactor() const
{
  return(usingFactor());
}

