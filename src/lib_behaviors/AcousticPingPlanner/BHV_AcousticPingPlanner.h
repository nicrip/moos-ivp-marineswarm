/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Inspired by Map_PingFormation by Simon Rohou    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AcousticPingPlanner.h                       */
/*    DATE:                                                 */
/************************************************************/

#ifndef AcousticPingPlanner_HEADER
#define AcousticPingPlanner_HEADER

#include <string>
#include "ManageAcousticPing/BHV_ManageAcousticPing.h"

class BHV_AcousticPingPlanner : public BHV_ManageAcousticPing {
public:
  BHV_AcousticPingPlanner(IvPDomain);
  ~BHV_AcousticPingPlanner() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();

protected: // Local Utility functions

protected: // Configuration parameters
  std::map<std::string, XYPoint> m_swarm_plan;  // stores the swarm plan (all node offsets)
  std::map<std::string, XYPoint> m_original_swarm_plan;     // stores the original position swarm plan

protected: // State variables
};

#endif
