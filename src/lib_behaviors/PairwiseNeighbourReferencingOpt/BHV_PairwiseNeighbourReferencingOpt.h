/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_PairwiseNeighbourReferencingOpt.h           */
/*    DATE:                                                 */
/************************************************************/

#ifndef PairwiseNeighbourReferencingOpt_HEADER
#define PairwiseNeighbourReferencingOpt_HEADER

#include <string>
#include <sstream>
#include "XYPolygon.h"
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_PairwiseNeighbourReferencingOpt : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  typedef struct {
    std::vector<double> weights;
    std::vector<double> mids_x;
    std::vector<double> mids_y;
    std::vector<double> mids_dist;
  } weights_mids_dist;

public:
  BHV_PairwiseNeighbourReferencingOpt(IvPDomain);
  ~BHV_PairwiseNeighbourReferencingOpt() {};

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
  void          optimization();
  void          postViewablePoints();
  void          postErasablePoints();

protected: // Configuration parameters
  double        m_contact_rangeout;
  bool          m_display_unaveraged_targets;
  bool          m_display_unaveraged_hull;
  std::string   m_weight_targets;

protected: // State variables
  double        m_rel_target_x;
  double        m_rel_target_y;
  double        m_rel_target_x_prev;
  double        m_rel_target_y_prev;
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
  weights_mids_dist m_weights_mids_dist;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_PairwiseNeighbourReferencingOpt(domain);}
}
#endif
