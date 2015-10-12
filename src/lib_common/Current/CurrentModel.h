/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentModel.h                                       */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_MODEL_HEADER
#define CURRENT_MODEL_HEADER

#include <iostream>
#include <string>
#include <vector>
#include <utility>

class CurrentModel
{
  public:
    CurrentModel();
    ~CurrentModel();
    void setActive();
    void setInactive();
    void switchActivation();
    bool isActive();
    double getStartingTime();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    virtual bool getCurrent(double x, double y, double depth,
                            double time,
                            double& drift_x,
                            double& drift_y,
                            double& drift_z) = 0;
    virtual std::string getType() = 0;
    virtual std::string getName();
    virtual void setParam(std::string param, std::string value) = 0;
    virtual bool init() = 0;
    
    static bool getCurrentFromListOfModels(std::vector<CurrentModel*> current_models, 
                                           double x, double y, double depth, double time,
                                           double& drift_x, double& drift_y, double& drift_z);

  protected:
    bool m_current_active;
    double m_starting_time;
    std::string m_name;
};

#endif
