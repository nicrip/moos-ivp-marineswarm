/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: RandomCurrent.h                                      */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_RANDOM_HEADER
#define CURRENT_RANDOM_HEADER

#include <string>
#include <utility>
#include "CurrentModel.h"

class RandomCurrent : public CurrentModel
{
  public:
    RandomCurrent();
    ~RandomCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
    void setSourceGap(double gap);
    void setSourcePeriod(int period);
    void setMaxSpeed(double max_speed);
                      
  protected:
    double m_max_speed;
    double m_source_gap;
    int m_source_period;
    std::pair<double,double> **m_current_grid;
};
#endif
