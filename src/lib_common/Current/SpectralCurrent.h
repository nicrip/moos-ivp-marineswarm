/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SpectralCurrent.h                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_SPECTRAL__HEADER
#define CURRENT_SPECTRAL__HEADER

#include <string>
#include "CurrentModel.h"

class SpectralCurrent : public CurrentModel
{
  public:
    SpectralCurrent();
    SpectralCurrent(double wavenumber, double SSH_amplitude);
    ~SpectralCurrent();
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
    void setWavenumber(double value);
    void setSSHAmplitude(double value);
    void setCoriolisParameter(double value);
    void setGravityConstant(double value);
    
  protected:
    double m_angle;
    double m_wavenumber;      // cycles/km
    double m_SSH_amplitude;   // centimeters
    double m_coriolis_parameter;
    double m_gravity_constant;
    double m_phase_x;
    double m_phase_y;
};
#endif
