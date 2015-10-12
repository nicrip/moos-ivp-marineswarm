/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: GridCurrent.cpp                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <math.h>
#include <iostream>
#include <algorithm>
#include "MBUtils.h"
#include "AngleUtils.h"
#include "GridCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

GridCurrent::GridCurrent()
{
  m_source_period = 0;
  m_source_gap = 0.;
}


//---------------------------------------------------------
// Procedure: Destructor

GridCurrent::~GridCurrent()
{
  for(int i = 0; i < m_source_period ; i++)
    free(m_current_grid[i]);
  
  free(m_current_grid);
}


//---------------------------------------------------------
// Procedure: setParam

void GridCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "GRID__SOURCE_GAP")
    setSourceGap(atof(value.c_str()));
    
  if(param == "GRID__SOURCE_PERIOD")
    setSourcePeriod(atoi(value.c_str()));
  
  if(param == "GRID__SOURCE_POINT")
    m_source_points.push_back(value);
}


//---------------------------------------------------------
// Procedure: init

bool GridCurrent::init()
{
  if(m_source_period <= 0 || m_source_gap <= 0)
    return false;
  
  srand(time(NULL));
  m_current_grid = new pair<double,double>*[m_source_period];
  for(int i = 0; i < m_source_period ; i++)
  {
    m_current_grid[i] = new pair<double,double>[m_source_period];
    for(int j = 0 ; j < m_source_period ; j++)
    {
      m_current_grid[i][j].first = 0;
      m_current_grid[i][j].second = 0;
    }
  }
  
  while (!m_source_points.empty())
  {
    string source_point = m_source_points.back();
    m_source_points.pop_back();
    handleSourcePoint(source_point);
  }
  return true;
}


//---------------------------------------------------------
// Procedure: setSourcePeriod

void GridCurrent::setSourcePeriod(int source_period)
{
  m_source_period = source_period;
}


//---------------------------------------------------------
// Procedure: setGridDimension

void GridCurrent::setSourceGap(double source_gap)
{
  m_source_gap = source_gap;
}


//---------------------------------------------------------
// Procedure: type

std::string GridCurrent::getType()
{
  return "GRID";
}


//---------------------------------------------------------
// Procedure: handleSourcePoint

void GridCurrent::handleSourcePoint(string value)
{
  string sub_values[4] = {"","","",""};
  
  int k = 0;
  for(int i = 0 ; i < value.length() ; i++)
  {
    if(value[i] == ',')
    {
      k++;
      continue;
    }
    
    sub_values[k] += value[i];
  }
  int i      = atoi(sub_values[0].c_str());
  int j      = atoi(sub_values[1].c_str());
  double rho    = atof(sub_values[2].c_str());
  double theta  = atof(sub_values[3].c_str());

  setSourcePoint(i,j,rho,theta);
  cout << "source point has been set" << endl;
}


//---------------------------------------------------------
// Procedure: setSourcePoint

void GridCurrent::setSourcePoint(int i, int j, double rho, double theta)
{
  if (i < m_source_period && j < m_source_period)
  {
    m_current_grid[i][j].first = sin(degToRadians(theta)) * rho;    // u
    m_current_grid[i][j].second = cos(degToRadians(theta)) * rho;   // v
  }
  else
    cout << "Source point i = " << i << " ; j = " << j << " is outside the original m_source_period" << endl;
}


//---------------------------------------------------------
// Procedure: getCurrent

bool GridCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  double grid_dimension = (m_source_period) * m_source_gap;
  
  double x_positive = x;
  while(x_positive < 0)
    x_positive += grid_dimension;
    
  double y_positive = y;
  while(y_positive < 0)
    y_positive += grid_dimension;
  
  // Position inside the first grid
  double x_first_grid = fmod(x_positive, grid_dimension);
  double y_first_grid = fmod(y_positive, grid_dimension);
  
  double gap = grid_dimension / m_source_period;
  
  // Retrieving bottom left point
  int i_A = (int)(x_first_grid / m_source_gap);
  int j_A = (int)(y_first_grid / m_source_gap);
  
  // Retrieving bottom right point
  int i_D = (i_A + 1) % m_source_period;
  int j_D = j_A;
  
  // Retrieving top right point
  int i_C = (i_A + 1) % m_source_period;
  int j_C = (j_A + 1) % m_source_period;
  
  // Retrieving top left point
  int i_B = i_A % m_source_period;
  int j_B = (j_A + 1) % m_source_period;
  
  // Ratio along axes
  double x_ratio = (x_first_grid - (i_A * m_source_gap)) / m_source_gap;
  double y_ratio = (y_first_grid - (j_A * m_source_gap)) / m_source_gap;
  
  // 4 modes
  
  // Mode avec poids en A
  
  #if 0 // linear calculations
    // A mode
    double ratio_AB = 1. - y_ratio;
    double ratio_AD = 1. - x_ratio;
    
    // B mode
    double ratio_BA = y_ratio;
    double ratio_BC = 1. - x_ratio;
    
    // C mode
    double ratio_CB = x_ratio;
    double ratio_CD = y_ratio;
    
    // D mode
    double ratio_DC = 1. - y_ratio;
    double ratio_DA = x_ratio;
  #endif
  
  #if 1 // cosinus calculations (smooth)
    // A mode
    double ratio_AB = (cos(M_PI * y_ratio) + 1) / 2.;
    double ratio_AD = (cos(M_PI * x_ratio) + 1) / 2.;
    
    // B mode
    double ratio_BA = (cos(M_PI * (1. - y_ratio)) + 1) / 2.;
    double ratio_BC = (cos(M_PI * x_ratio) + 1) / 2.;
    
    // C mode
    double ratio_CB = (cos(M_PI * (1. - x_ratio)) + 1) / 2.;
    double ratio_CD = (cos(M_PI * (1. - y_ratio)) + 1) / 2.;
    
    // D mode
    double ratio_DC = (cos(M_PI * y_ratio) + 1) / 2.;
    double ratio_DA = (cos(M_PI * (1. - x_ratio)) + 1) / 2.;
  #endif
  
  // Fusion of weights
  drift_x = ratio_AB * ratio_AD * m_current_grid[i_A][j_A].first + 
            ratio_BA * ratio_BC * m_current_grid[i_B][j_B].first + 
            ratio_CB * ratio_CD * m_current_grid[i_C][j_C].first + 
            ratio_DC * ratio_DA * m_current_grid[i_D][j_D].first;
                    
  drift_y = ratio_AB * ratio_AD * m_current_grid[i_A][j_A].second + 
            ratio_BA * ratio_BC * m_current_grid[i_B][j_B].second + 
            ratio_CB * ratio_CD * m_current_grid[i_C][j_C].second + 
            ratio_DC * ratio_DA * m_current_grid[i_D][j_D].second;
  
  drift_z = 0.;
  
  return true;
}
