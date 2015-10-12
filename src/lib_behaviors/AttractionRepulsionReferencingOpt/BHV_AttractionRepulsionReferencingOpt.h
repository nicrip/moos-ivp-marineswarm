/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsionReferencingOpt.h         */
/*    DATE:                                                 */
/************************************************************/

#ifndef AttractionRepulsionReferencingOpt_HEADER
#define AttractionRepulsionReferencingOpt_HEADER

#include <string>
#include <sstream>
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_AttractionRepulsionReferencingOpt : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  typedef struct {
    std::vector<double> weights;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> dists;
  } weights_centers_dists;

public:
  BHV_AttractionRepulsionReferencingOpt(IvPDomain);
  ~BHV_AttractionRepulsionReferencingOpt() {};

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
  double        m_force_rangeout;
  std::string   m_weight_targets;

protected: // State variables
  double        m_rel_target_x;
  double        m_rel_target_y;
  double        m_rel_target_x_prev;
  double        m_rel_target_y_prev;
  XYPoint       m_abs_target;
  double        m_distance_weight;
  double        m_pingtime_weight;
  bool          m_weight_distance;
  bool          m_weight_pingtime;
  double        m_total_weight;
  int           m_num_contacts;
  int           m_num_contacts_prev;
  std::ostringstream m_warning_message;
  weights_centers_dists m_weights_centers_dists;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_AttractionRepulsionReferencingOpt(domain);}
}
#endif
