/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AttractionRepulsionOpt.h                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef AttractionRepulsionOpt_HEADER
#define AttractionRepulsionOpt_HEADER

#include <string>
#include <sstream>
#include <deque>
#include "XYPolygon.h"
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

class BHV_AttractionRepulsionOpt : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  typedef struct {
    std::vector<double> weights;
    std::vector<double> x;
    std::vector<double> y;
    double centroid_x;
    double centroid_y;
    double dist;
  } weights_centers_dist;

public:
  BHV_AttractionRepulsionOpt(IvPDomain);
  ~BHV_AttractionRepulsionOpt() {};

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
  double        m_separation_distance;
  int           m_max_num_neighbours;
  int           m_target_avg_length;
  bool          m_display_neighbour_hull;

protected: // State variables
  double        m_rel_target_x;
  double        m_rel_target_y;
  double        m_rel_target_x_prev;
  double        m_rel_target_y_prev;
  std::deque<double> m_rel_target_x_deque;
  std::deque<double> m_rel_target_y_deque;
  int           m_deque_size;
  XYPoint       m_abs_target;
  double        m_distance_weight;
  double        m_pingtime_weight;
  bool          m_weight_distance;
  bool          m_weight_pingtime;
  double        m_total_weight;
  int           m_num_contacts;
  int           m_num_nbrs;
  int           m_num_nbrs_prev;
  double        m_centroid_x;
  double        m_centroid_y;
  std::ostringstream m_warning_message;
  weights_centers_dist m_weights_centers_dist;
  std::vector< std::pair< double, std::pair<double, double> > > m_dist_x_y;
  XYPolygon     m_abs_targets;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_AttractionRepulsionOpt(domain);}
}
#endif
