/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: RandomCurrent.cpp                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <math.h>
#include <iostream>
#include <algorithm>
#include "RandomCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

RandomCurrent::RandomCurrent()
{
  m_source_period = 0;
  m_source_gap = 0.;
  m_max_speed = 0.;
}


//---------------------------------------------------------
// Procedure: Destructor

RandomCurrent::~RandomCurrent()
{
  for(int i = 0; i < m_source_period ; i++)
    free(m_current_grid[i]);
  
  free(m_current_grid);
}


//---------------------------------------------------------
// Procedure: setParam

void RandomCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "RANDOM__SOURCE_GAP")
    setSourceGap(atof(value.c_str()));
    
  if(param == "RANDOM__SOURCE_PERIOD")
    setSourcePeriod(atoi(value.c_str()));
    
  if(param == "RANDOM__MAX_SPEED")
    setMaxSpeed(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool RandomCurrent::init()
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
      double rho = ((double)(rand() % 10000) / 10000.) * m_max_speed;
      double theta = ((double)(rand() % 10000) / 10000.) * 2 * M_PI;
      m_current_grid[i][j].first = cos(theta) * rho;
      m_current_grid[i][j].second = sin(theta) * rho;
    }
  }
  
  return true;
}


//---------------------------------------------------------
// Procedure: setMaxSpeed

void RandomCurrent::setMaxSpeed(double max_speed)
{
  m_max_speed = max_speed;
}


//---------------------------------------------------------
// Procedure: setSourcePeriod

void RandomCurrent::setSourcePeriod(int source_period)
{
  m_source_period = source_period;
}


//---------------------------------------------------------
// Procedure: setGridDimension

void RandomCurrent::setSourceGap(double source_gap)
{
  m_source_gap = source_gap;
}


//---------------------------------------------------------
// Procedure: type

std::string RandomCurrent::getType()
{
  return "RANDOM";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool RandomCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  double grid_dimension = m_source_period * m_source_gap;
  
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
