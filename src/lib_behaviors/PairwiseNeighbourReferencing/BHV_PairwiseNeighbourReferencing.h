/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_PairwiseNeighbourReferencing.h              */
/*    DATE:                                                 */
/************************************************************/

#ifndef PairwiseNeighbourReferencing_HEADER
#define PairwiseNeighbourReferencing_HEADER

#include <string>
#include <sstream>
#include "XYPolygon.h"
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_PairwiseNeighbourReferencing : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  BHV_PairwiseNeighbourReferencing(IvPDomain);
  ~BHV_PairwiseNeighbourReferencing() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  void          postViewablePoints();
  void          postErasablePoints();

protected: // Configuration parameters
  double        m_contact_rangeout;
  bool          m_display_unaveraged_targets;
  bool          m_display_unaveraged_hull;
  std::string   m_weight_targets;
  bool          m_weight_averaging;

protected: // State variables
  double        m_rel_target_x;
  double        m_rel_target_y;
  XYPoint       m_abs_target;
  XYPolygon     m_abs_targets;
  double        m_distance_weight;
  double        m_pingtime_weight;
  bool          m_weight_distance;
  bool          m_weight_pingtime;
  double        m_total_weight;
  int           m_num_target_pairs;
  int           m_num_target_pairs_prev;
  std::ostringstream m_warning_message;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_PairwiseNeighbourReferencing(domain);}
}
#endif
