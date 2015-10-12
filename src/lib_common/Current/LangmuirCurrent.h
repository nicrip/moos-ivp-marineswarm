/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: LangmuirCurrent.h                                    */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_LANGMUIR_HEADER
#define CURRENT_LANGMUIR_HEADER

#include <list>
#include <string>
#include "CurrentModel.h"

//---------------------------------------------------------
// Structure: Cell

struct cell
{
  double  xmin;     
  double  xmax;
  double  yaxis;
  double  depthaxis;
  double  radius;
  bool    circ;   //rotation direction
};

//---------------------------------------------------------
// Class : LangmuirCurrent

class LangmuirCurrent : public CurrentModel
{
  public:
    LangmuirCurrent();
    ~LangmuirCurrent();
    bool init();
    std::string getType();
    // drift x, drift y and drift z are expressed in the EAST-NORTH-UP coordinate system
    bool getCurrent(double x, double y, double depth,
                      double time,
                      double& drift_x,
                      double& drift_y,
                      double& drift_z);
    void setParam(std::string param, std::string value);
    //setters
    void setRange(double value);
    void setMaxSpeed(double value);
    void setOperationDepth(double value);
    void setCellLength(double value);
    void setWindDirection(double value);
    void setPerc10(double value);
  
  protected: //used in getCurrent
    bool findClosestCells(double x, double y, double depth, 
                          cell& cellOne, 
                          cell& cellTwo,
                          double& distOne,
                          double& distTwo);
    double findAmplitude(double dist, double radius);
    void normalUnitVector(double a, double b, double& u, double& v, bool circ);
  
  protected:
    double m_range;             //meters
    double m_max_speed;         //meters/s
    double m_operation_depth;   //meters
    double m_cell_length;       //meters
    double m_wind_direction;    //radians
    double m_perc_10;
    
    std::list<std::list<cell> >  m_cells;
};

#endif
