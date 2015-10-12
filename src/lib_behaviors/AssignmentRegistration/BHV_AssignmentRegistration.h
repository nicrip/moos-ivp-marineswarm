/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_AssignmentRegistration.h                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef AssignmentRegistration_HEADER
#define AssignmentRegistration_HEADER

#include <string>
#include <sstream>
#include <armadillo>
#include "XYPolygon.h"
#include "DriftingTarget/BHV_DriftingTarget.h"
#include "AcousticPingPlanner/BHV_AcousticPingPlanner.h"

using namespace arma;

class BHV_AssignmentRegistration : public BHV_DriftingTarget, public BHV_AcousticPingPlanner {
public:
  BHV_AssignmentRegistration(IvPDomain);
  ~BHV_AssignmentRegistration() {};

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
  void          sortFormationPoints(mat formation, std::vector<mat> &sorted_formation, std::vector<vec> &sorted_dists);
  void          assignmentScores(mat nodes, int angle_delta, std::vector<mat> &scores_form, std::vector<umat> &assignments, colvec &min_costs);
  void          optimalAssignment(std::vector<mat> &scores_form, std::vector<umat> &assignments, colvec &min_costs, mat &optimal_formation);
  void          postViewablePoints();
  void          postErasablePoints();

  arma::umat hungarian(const arma::mat &input_cost);
  void step_six (unsigned int &step, arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N);
  void find_smallest (double &minval, const arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N);
  void step_five (unsigned int &step,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, arma::imat &path,
        int &rpath_0, int &cpath_0,
        const unsigned int &N);
  void erase_primes(arma::umat &indM, const unsigned int &N);
  void clear_covers (arma::ivec &rcov, arma::ivec &ccov);
  void augment_path (const int &path_count, arma::umat &indM,
        const arma::imat &path);
  void find_prime_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N);
  void find_star_in_col (const int &col, int &row,
        const arma::umat &indM, const unsigned int &N);
  void step_four (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov, arma::ivec &ccov,
        int &rpath_0, int &cpath_0, const unsigned int &N);
  void find_star_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N);
  bool star_in_row(int &row, const arma::umat &indM,
        const unsigned int &N);
  void find_noncovered_zero(int &row, int &col,
        const arma::mat &cost, const arma::ivec &rcov,
        const arma::ivec &ccov, const unsigned int &N);
  void step_three(unsigned int &step, const arma::umat &indM,
        arma::ivec &ccov, const unsigned int &N);
  void step_two (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, const unsigned int &N);
  void step_one(unsigned int &step, arma::mat &cost,
        const unsigned int &N);

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
  std::vector<mat> m_sorted_formation_points;
  std::vector<vec> m_sorted_formation_dists;
  mat           m_nodes_points;
//  double        m_init_angle;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain)
  {return new BHV_AssignmentRegistration(domain);}
}
#endif
