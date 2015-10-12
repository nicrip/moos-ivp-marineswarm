/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: ShearCurrent.h                                       */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_SHEAR_HEADER
#define CURRENT_SHEAR_HEADER

#include <string>
#include "CurrentModel.h"

class ShearCurrent : public CurrentModel
{
  public:
    ShearCurrent();
    ~ShearCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
    void setGap(double gap);
    void setTheta(double theta);
    void setMaxCurrentSpeed(double speed);
    
  protected:
    double m_gap;
    double m_theta;
    double m_max_current_speed;
};
#endif
