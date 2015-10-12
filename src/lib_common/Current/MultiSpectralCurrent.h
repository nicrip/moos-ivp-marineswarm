/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: MultiSpectralCurrent.h                               */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_MULTI_SPECTRAL__HEADER
#define CURRENT_MULTI_SPECTRAL__HEADER

#include <string>
#include "CurrentModel.h"
#include "SpectralCurrent.h"

class MultiSpectralCurrent : public CurrentModel
{
  public:
    MultiSpectralCurrent();
    ~MultiSpectralCurrent();
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
    void setCoriolisParameter(double value);
    void setGravityConstant(double value);
    
  protected:
    double m_coriolis_parameter;
    double m_gravity_constant;
    std::vector<SpectralCurrent*> m_vector_spectral_currents;
};
#endif
