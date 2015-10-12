/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: GridCurrent.h                                      */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_GRID_HEADER
#define CURRENT_GRID_HEADER

#include <string>
#include <utility>
#include "MOOS/libMOOS/MOOSLib.h"
#include "CurrentModel.h"

class GridCurrent : public CurrentModel
{
  public:
    GridCurrent();
    ~GridCurrent();
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
    void handleSourcePoint(std::string value);
    void setSourcePoint(int i, int j, double rho, double theta);
                      
  protected:
    double m_source_gap;
    int m_source_period;
    std::pair<double,double> **m_current_grid;
    std::vector<std::string> m_source_points;
};
#endif
