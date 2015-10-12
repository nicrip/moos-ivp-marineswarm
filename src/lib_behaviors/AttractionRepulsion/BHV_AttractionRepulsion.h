/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsion.h                       */
/*    DATE:                                                 */
/************************************************************/

#ifndef AttractionRepulsion_HEADER
#define AttractionRepulsion_HEADER

#include <string>
#include <sstream>
#include "XYCircle.h"
#include "DriftingForce/BHV_DriftingForce.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_AttractionRepulsion : public BHV_DriftingForce, public BHV_AcousticPingPlanner {
public:
  BHV_AttractionRepulsion(IvPDomain);
  ~BHV_AttractionRepulsion() {};

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
  void          attractionRepulsionAtomic(double dist, double heading, double& force_x, double& force_y);
  void          attractionRepulsionPiecewiseLinear(double dist, double heading, double& force_x, double& force_y);
  void          postViewablePoints();
  void          postErasablePoints();

protected: // Configuration parameters
  double        m_contact_rangeout;
  bool          m_display_targets;
  std::string   m_weight_targets;
  bool          m_averaging;
  bool          m_weight_averaging;

protected: // State variables
  double        m_total_force_x;
  double        m_total_force_y;
  XYPoint       m_abs_target;
  XYCircle      m_abs_circle;
  XYPolygon     m_abs_targets;
  double        m_distance_weight;
  double        m_pingtime_weight;
  bool          m_weight_distance;
  bool          m_weight_pingtime;
  double        m_total_weight;
  int           m_num_targets;
  int           m_num_targets_prev;
  std::ostringstream m_warning_message;
  ColorPack     m_color;
  double        m_zero_crossing;
  double        m_piece1_gradient;
  double        m_piece2_diff_x;
  double        m_piece2_diff_y;
  double        m_piece3_diff_x;
  double        m_piece3_diff_y;
  double        m_piece4_diff_x;
  double        m_piece4_diff_y;
  double        m_piece5_diff_x;
  double        m_piece5_diff_y;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_AttractionRepulsion(domain);}
}
#endif
