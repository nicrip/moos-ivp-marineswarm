/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SolitonCurrent.h                                     */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_SOLITON_HEADER
#define CURRENT_SOLITON_HEADER

#include <string>
#include "CurrentModel.h"

class SolitonCurrent : public CurrentModel
{
  public:
    SolitonCurrent();
    ~SolitonCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
                      
  protected: //used in getCurrent
    bool parametersOutOfDate (double time);
    void setNewParameters();
    
  public: //setters
    void setWavelength(double value);
    void setMaxHorizontalSpeed(double value);
    void setMaxVerticalAmplitude(double value);
    void setWavePeriod(double value);
    void setNumberOfWaves(int value);
    void setFirstFrontAtZero(double time);
    void setTideDirection(double value); // set in degrees, m_tide_direction in radiands
    
    
  protected:
    double m_wavelength;
    double m_max_horizontal_speed;
    double m_max_vertical_amplitude;
    double m_wave_period;
    double m_number_of_waves;
    double m_first_front_at_zero;
    double m_tide_direction;
};
#endif
