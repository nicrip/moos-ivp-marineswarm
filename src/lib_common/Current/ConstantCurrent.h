/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: ConstantCurrent.h                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_CONSTANT_HEADER
#define CURRENT_CONSTANT_HEADER

#include <string>
#include "CurrentModel.h"

class ConstantCurrent : public CurrentModel
{
  public:
    ConstantCurrent();
    ~ConstantCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
    void setDriftX(double drift);
    void setDriftY(double drift);
    void setDriftZ(double drift);
                      
  protected:
    double m_drift_x, m_drift_y, m_drift_z;
};
#endif
