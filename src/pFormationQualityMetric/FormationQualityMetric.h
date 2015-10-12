/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: FormationQualityMetric.h                        */
/*    DATE:                                                 */
/************************************************************/

#ifndef FormationQualityMetric_HEADER
#define FormationQualityMetric_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <map>
#include "NodeRecord.h"
#include <armadillo>
#include "XYPoint.h"
#include <sstream>
using namespace arma;

class FormationQualityMetric : public CMOOSApp
{
 public:
   FormationQualityMetric();
   ~FormationQualityMetric();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool HandleNewNodeReport(CMOOSMsg msg);
   void rigidTransformation2D(mat p, mat q, mat cent_p, mat cent_q,  mat &R, mat &T, mat &sol);
   void assignmentScores(int angle_delta, umat &min_assignment);
   void metricScore(mat &opt_met, mat &curr_pos);

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

 private: // Configuration variables
   bool         m_display_rigid_registration;
   std::vector<XYPoint> m_vector_node_offsets;
   mat          m_node_offsets;
   mat          m_centroid_node_offsets;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   std::map< std::string, NodeRecord > m_map_node_records;    // <vehicle_name, node_record>
   int          m_num_communities;
   mat          m_curr_node_offsets;
   mat          m_centroid_curr_node_offsets;
   umat         m_min_assignment;
   mat          m_optimal_metric;
   int          m_num_nodes;
   int          m_num_nodes_prev;
   XYPoint      m_abs_target;
   std::ostringstream m_warning_message;
   double       m_metric_score;
   std::vector<std::string> m_ignored_nodes;
};

#endif
