/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: VortexCurrent.h                                      */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_VORTEX__HEADER
#define CURRENT_VORTEX__HEADER

#include <string>
#include "CurrentModel.h"

class VortexCurrent : public CurrentModel
{
  public:
    VortexCurrent();
    ~VortexCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
    
  public: //setters
    void setPosition(double x, double y);
    void setMagnitude(double value);
    void setRadius(double value);
    
  protected:
    double m_position_x;
    double m_position_y;
    double m_magnitude;
    double m_radius;
};
#endif
