/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTargetWaypoint.h                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingTargetWaypoint_HEADER
#define DriftingTargetWaypoint_HEADER

#include <string>
#include "DriftingTarget/BHV_DriftingTarget.h"

class BHV_DriftingTargetWaypoint : public BHV_DriftingTarget {
public:
  BHV_DriftingTargetWaypoint(IvPDomain);
  ~BHV_DriftingTargetWaypoint() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_wpt_x;
  double m_wpt_y;

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_DriftingTargetWaypoint(domain);}
}

#endif
