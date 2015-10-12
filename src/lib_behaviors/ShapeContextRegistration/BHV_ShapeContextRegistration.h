/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ShapeContextRegistration.h                  */
/*    DATE:                                                 */
/************************************************************/

#ifndef ShapeContextRegistration_HEADER
#define ShapeContextRegistration_HEADER

#include <string>
#include <sstream>
#include <armadillo>
#include "XYPolygon.h"
#include "Hungarian/hungarian.h"
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"
using namespace arma;

class BHV_ShapeContextRegistration :  public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  BHV_ShapeContextRegistration(IvPDomain);
  ~BHV_ShapeContextRegistration() {};

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
  void          rigidTransformation2D(mat p, mat q, mat w, mat cent_p, mat cent_q,  mat &R, mat &T, mat &sol);
  void          shapeContexts(mat f, double ang_delta, double max_dist,  std::vector< std::vector< std::vector<double> > > &shape_contexts);
  void          shapeContextScores(std::vector< std::vector<double> > &shape_context_node, std::vector< std::vector<double> > &shape_context_formation, Cube<int> &scores);
  int**         matToArrayArray(Mat<int> &m, int rows, int cols);
//  double          principalComponentDifference(mat f, mat n);
  void          optimalCorrespondences(mat &new_formation_points);
  void          postViewablePoints();
  void          postErasablePoints();

protected: // Configuration parameters
  double        m_contact_rangeout;
  bool          m_display_rigid_registration_points;
  bool          m_display_rigid_registration_hull;
  std::string   m_weight_targets;
  bool          m_weight_averaging;
  double        m_ownship_weight;

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
  int           m_num_contacts;
  int           m_num_contacts_prev;
  std::ostringstream m_warning_message;
  mat           m_formation_points;
  std::vector< std::vector< std::vector<double> > > m_formation_shape_contexts;
  mat           m_nodes_points;
  std::vector< std::vector< std::vector<double> > > m_nodes_shape_contexts;
  hungarian_problem_t m_hungarian;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_ShapeContextRegistration(domain);}
}
#endif
