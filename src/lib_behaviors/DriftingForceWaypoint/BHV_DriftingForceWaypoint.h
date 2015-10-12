/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForceWaypoint.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingForceWaypoint_HEADER
#define DriftingForceWaypoint_HEADER

#include <string>
#include "DriftingForce/BHV_DriftingForce.h"
#include "XYCircle.h"
#include "XYPoint.h"

class BHV_DriftingForceWaypoint : public BHV_DriftingForce {
public:
  BHV_DriftingForceWaypoint(IvPDomain);
  ~BHV_DriftingForceWaypoint() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  IvPFunction* onRunState();

protected: // Local Utility functions
  void        attractionConstantRepulsionUnbounded(double r);
  void        attractionLinearRepulsionUnbounded(double r);
  void        attractionLinearRepulsionBounded(double r);

protected: // Configuration parameters
  double m_force;
  double m_dist;
  double m_force_heading;
  double m_wpt_x;
  double m_wpt_y;
  double m_curr_wpt_x;
  double m_curr_wpt_y;
  XYCircle m_circle;
  XYCircle m_circle2;
  std::list<XYPoint> m_wpts;

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_DriftingForceWaypoint(domain);}
}
#endif
