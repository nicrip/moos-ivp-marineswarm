/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentModel.cpp                                     */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include "MBUtils.h"
#include "CurrentModel.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

CurrentModel::CurrentModel()
{
  m_current_active = true;
  m_name = "unnamed";
}


//---------------------------------------------------------
// Procedure: Destructor

CurrentModel::~CurrentModel()
{
  
}


//---------------------------------------------------------
// Procedure: getCurrentFromListOfModels

bool CurrentModel::getCurrentFromListOfModels(vector<CurrentModel*> current_models, 
                                              double x, double y, double depth, double time,
                                              double& drift_x, double& drift_y, double& drift_z)
{
  double one_drift_x, one_drift_y, one_drift_z;
  drift_x = 0.;
  drift_y = 0.;
  drift_z = 0.;
  
  if(current_models.size() == 0)
    return true;
  
  for(vector<CurrentModel*>::iterator model = current_models.begin() ; 
      model != current_models.end() ; 
      model++)
  {
    if((*model)->isActive())
    {
      if(!(*model)->getCurrent(x, y, depth, time, one_drift_x, one_drift_y, one_drift_z))
        return false;
        
      drift_x += one_drift_x;
      drift_y += one_drift_y;
      drift_z += one_drift_z;
    }
  }
  
  return true;
}


//---------------------------------------------------------
// Procedure: getStartingTime

double CurrentModel::getStartingTime()
{
  return m_starting_time;
}


//---------------------------------------------------------
// Procedure: isActive

bool CurrentModel::isActive()
{
  return m_current_active;
}


//---------------------------------------------------------
// Procedure: setActive

void CurrentModel::setActive()
{
  m_current_active = true;
}


//---------------------------------------------------------
// Procedure: setActive

void CurrentModel::setInactive()
{
  m_current_active = false;
}


//---------------------------------------------------------
// Procedure: switchActivation

void CurrentModel::switchActivation()
{
  m_current_active = !m_current_active;
}


//---------------------------------------------------------
// Procedure: getName

string CurrentModel::getName()
{
  return m_name;
}


//---------------------------------------------------------
// Procedure: setParam

void CurrentModel::setParam(string param, string value)
{
  if(param == "CURRENT_NAME")
    m_name = value;
    
  if(param == "ACTIVE")
    setBooleanOnString(m_current_active, value);
}

