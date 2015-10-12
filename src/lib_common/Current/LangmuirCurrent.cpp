/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: LangmuirCurrent.cpp                                  */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <iostream> // cout
#include <cmath>
#include <list>
#include <stdlib.h>     // rand()
#include "LangmuirCurrent.h"
#include "AngleUtils.h"

using namespace std;

//---------------------------------------------------------
// Procedure: Constructor

LangmuirCurrent::LangmuirCurrent()
{
  m_range           = 10000;  //10km
  m_operation_depth = 200;    //200m
  m_cell_length     = 150;    //150m
  m_max_speed       = 0.2;    //20 cm/s
  m_wind_direction  = 0;      //in radians. WARNING : m_wind_direction is given from x-axis to y-axis, not clockwise from North !
  m_perc_10         = 1;      //means that the current is 10% of max speed after 1 * radius outside the cell
}
  
//---------------------------------------------------------
// Procedure: Destructor

LangmuirCurrent::~LangmuirCurrent()
{
  
}


//---------------------------------------------------------
// Procedure: setParam

void LangmuirCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "LANGMUIR__RANGE")
    setRange(atof(value.c_str()));
  
  if(param == "LANGMUIR__OPERATION_DEPTH")
    setOperationDepth(atof(value.c_str()));
  
  if(param == "LANGMUIR__CELL_LENGTH")
    setCellLength(atof(value.c_str()));
  
  if(param == "LANGMUIR__MAX_SPEED")
    setMaxSpeed(atof(value.c_str()));
  
  if(param == "LANGMUIR__WIND_DIRECTION")
    setWindDirection(atof(value.c_str()));
  
  if(param == "LANGMUIR__PERC_10")
    setPerc10(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool LangmuirCurrent::init()
{
  if (m_cell_length <=0)
  {
    cout << "Error : CELL_LENGTH is negative" << endl;
    return false;
  }
  
  std::list<cell> currentColumn;
  
  // fill the first cell
  
  cell newCell;
  
  double radius = rand() % 20 + 5;
  double delta_depth = -radius + 2 * radius * ((rand() % 101) / 100);
  double xmin = -m_range;
  double xmax = -m_range + m_cell_length;
  double yaxis = -m_range - 2 * radius * ((rand() % 101) / 100);
  double depthaxis = m_operation_depth + delta_depth;
  bool circ = rand() % 2;
  
  newCell.xmin        = xmin;
  newCell.xmax        = xmax;
  newCell.yaxis       = yaxis;
  newCell.depthaxis   = depthaxis;
  newCell.radius      = radius;
  newCell.circ        = circ;
  
  currentColumn.push_back(newCell);
  
  // Populate m_cells

  while (xmin < m_range)
  {
      
    // we complete the column
    while (yaxis < m_range)
    {
      cell newCell;
      
      double old_radius = radius;
      double old_delta_depth = delta_depth;
      
      // new radius is picked
      radius = rand() % 20 + 5;     
      // cell is randomly vertically placed around operation depth
      delta_depth = -radius + 2 * radius * ((rand() % 101) / 100); 
      depthaxis = m_operation_depth + delta_depth;
      // cells are horizontally placed so that they 'touch' each other
      yaxis = yaxis + sqrt((old_radius + radius)*(old_radius + radius) - (old_delta_depth - delta_depth)*(old_delta_depth - delta_depth));    
      // alternate rotating direction
      circ = !circ;     
      
      newCell.xmin        = xmin;
      newCell.xmax        = xmax;
      newCell.yaxis       = yaxis;
      newCell.depthaxis   = depthaxis;
      newCell.radius      = radius;
      newCell.circ        = circ;
      
      currentColumn.push_back(newCell);
    }

    m_cells.push_back(currentColumn);
    
    // we start a new column
    currentColumn.clear();
    
    radius = rand() % 20 + 5;
    delta_depth = -radius + 2 * radius * ((rand() % 101) / 100);
    xmin = xmax;
    xmax = xmax + m_cell_length;
    yaxis = -m_range - 2 * radius * ((rand() % 101) / 100);
    depthaxis = m_operation_depth + delta_depth;
    circ = rand() % 2;
    
    newCell.xmin        = xmin;
    newCell.xmax        = xmax;
    newCell.yaxis       = yaxis;
    newCell.depthaxis   = depthaxis;
    newCell.radius      = radius;
    newCell.circ        = circ;
    
    currentColumn.push_back(newCell);
  }
  
  return true;
}

//---------------------------------------------------------
// Procedure: setRange

void LangmuirCurrent::setRange(double value)
{
  m_range = value;
}

//---------------------------------------------------------
// Procedure: setMaxSpeed

void LangmuirCurrent::setMaxSpeed(double value)
{
  m_max_speed = value;
}

//---------------------------------------------------------
// Procedure: type

std::string LangmuirCurrent::getType()
{
  return "LANGMUIR";
}


//---------------------------------------------------------
// Procedure: setOperationDepth

void LangmuirCurrent::setOperationDepth(double value)
{
  m_operation_depth = value;
}


//---------------------------------------------------------
// Procedure: setCellLength

void LangmuirCurrent::setCellLength(double value)
{
  m_cell_length = value;
}

//---------------------------------------------------------
// Procedure: setWindDirection

void LangmuirCurrent::setWindDirection(double value) //value is in degrees
{
  m_wind_direction = degToRadians(value);
}

//---------------------------------------------------------
// Procedure: setPerc10

void LangmuirCurrent::setPerc10(double value)
{
  m_perc_10 = value;
}

//---------------------------------------------------------
// Procedure: findClosestCells
//
// finds the two closest cell axes in m_cells

bool LangmuirCurrent::findClosestCells(double x, double y, double depth, cell& cellOne, cell& cellTwo, double& distOne, double& distTwo)
{
  //if out of range (never happens if we apply %modulo% m_range at x and y first)
  if (abs(x) > m_range || abs(y) > m_range)
    return false;
  
  distOne = 1000.; //large distances defined on purpose (not meant to be kept)
  distTwo = 1000.;
    
  std::list<std::list<cell> >::iterator it_column = m_cells.begin();
  std::list<cell>::iterator it_cell = it_column->begin();
  
  // find the column
  double xmax = it_cell->xmax;
  
  while (x > xmax && it_column != m_cells.end())
  {
    it_column++;
    it_cell = it_column->begin();
    xmax = it_cell->xmax;
  }
  
  if (it_column == m_cells.end()) // x is too great, out of range
    return false;
  
  // find the closest cells in the column
  int found = 0; // number of cells we find closer
  
  while (it_cell != it_column->end())
  {
    xmax              = it_cell->xmax;
    double xmin       = it_cell->xmin;
    double yaxis      = it_cell->yaxis;
    double radius     = it_cell->radius;
    double depthaxis  = it_cell->depthaxis;
    
    double dist = hypot(y-yaxis, -depth+depthaxis) - radius;
    
    if (dist < distOne)
    {
      cellTwo = cellOne;
      distTwo = distOne;
      cellOne = *it_cell;
      distOne = dist;
      found++;
    }
    else if (dist < distTwo)
    {
      cellTwo = *it_cell;
      distTwo = dist;
      found++;
    }
    it_cell++;
  }
  
  return found > 1;
}

//---------------------------------------------------------
// Procedure: findAmplitude
//
// returns the current amplitude according to :
// * a linear profil for dist < 0, null for dist = -radius and m_max_speed for dist = 0
// * a decreasing exponential profil from dist = 0 to infinity, m_max_speed for dist = 0 and null to infinity
// * at dist = m_perc_10 * radius, only 10% of m_max_speed is remaining

double LangmuirCurrent::findAmplitude(double dist, double radius)
{
  double amplitude = 0;
  if (dist < 0)
    amplitude = m_max_speed * (1 + dist / radius);    // linear profil inside the circle
  else
  {
    double D = m_perc_10 * radius / log(10);
    amplitude = m_max_speed * exp(-dist / D);  // exponential profil outside the circle
  }
  return amplitude;
}

//---------------------------------------------------------
// Procedure: normalUnitVector
//
// builds a unit vector (u,v) normal to vector (a,b), 
// oriented anti-clockwise if circ = 0 and clockwise if circ = 1

void LangmuirCurrent::normalUnitVector(double a, double b, double& u, double& v, bool circ)
{
  double norm = hypot(a,b);
  // circ = 0, anti-clockwise, from y-axis to z-axis
  u = -b / norm;
  v = a / norm;
  // circ = 1, clockwise, from z-axis to y-axis
  if (circ==1)
  {
    u = -u;
    v = -v;
  }
}


//---------------------------------------------------------
// Procedure: getCurrent 
//
// Note : drift_x, drift_y and drift_z are expressed in the EAST-NORTH-UP coordinate system
// For now, m_wind_direction is constant

bool LangmuirCurrent::getCurrent(double x, double y, double depth, double time, double& drift_x, double& drift_y, double& drift_z)
{
  // rotate coordinates x,y,z around z-axis to X,Y,Z so that the wind is blowing in X-direction
  
  double X =  cos(m_wind_direction) * x + sin(m_wind_direction) * y;
  double Y = -sin(m_wind_direction) * x + cos(m_wind_direction) * y;
  // Z = z
  
  // if out of range, we 'copy' the position inside the range
  X = fmod(X,m_range);
  Y = fmod(Y,m_range);
  
  // look for the two closest cell axes
  
  cell cellOne;
  cell cellTwo;
  double distOne;
  double distTwo;
  
  if (!findClosestCells(X, Y, depth, cellOne, cellTwo, distOne, distTwo))
    return false;

  // From here, we work in the Y,Z plane since Langmuir current are perpendicular to the wind direction
  // calculate drift components from cellOne
  double xmin       = cellOne.xmin;
  double xmax       = cellOne.xmax;
  double yaxis      = cellOne.yaxis;
  double radius     = cellOne.radius;
  double depthaxis  = cellOne.depthaxis;
  double circ       = cellOne.circ;

  double amplitude = findAmplitude(distOne, radius);
  
  double u = 0;
  double v = 0;
  normalUnitVector(Y-yaxis, -depth+depthaxis, u, v, circ);
  
  double driftOneY = amplitude * u;
  double driftOneZ = amplitude * v;
  
  // calculate drift components from cellTwo
  xmin       = cellTwo.xmin;
  xmax       = cellTwo.xmax;
  yaxis      = cellTwo.yaxis;
  radius     = cellTwo.radius;
  depthaxis  = cellTwo.depthaxis;
  circ       = cellTwo.circ;

  amplitude = findAmplitude(distTwo, radius);
  
  normalUnitVector(Y-yaxis, -depth+depthaxis, u, v, circ);
  
  double driftTwoY = amplitude * u;
  double driftTwoZ = amplitude * v;
  
  // weighting drifts components
  // inside a cell : only the components from this cell
  // outside both cells : the components are weighted according to the distance to each cell
  
  double coefOne = distTwo / (distOne + distTwo);
  double coefTwo = 1 - coefOne;
  
  if (distOne < 0)      //inside cellOne
  {
    coefOne = 1;
    coefTwo = 0;
  }
  else if (distTwo < 0) //inside cellTwo
  {
    coefOne = 0;
    coefTwo = 1;
  }
  
  double drift_Y = coefOne * driftOneY + coefTwo * driftTwoY;
  double drift_Z = coefOne * driftOneZ + coefTwo * driftTwoZ;
  
  // project drift components to original coordinates x,y
  drift_x = -sin(m_wind_direction) * drift_Y;
  drift_y = cos(m_wind_direction) * drift_Y;
  drift_z = drift_Z;

  return true;
}
