/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: FormationQualityMetric.cpp                      */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <limits>
#include "MBUtils.h"
#include "FormationQualityMetric.h"
#include "NodeRecordUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FormationQualityMetric::FormationQualityMetric()
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

FormationQualityMetric::~FormationQualityMetric()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool FormationQualityMetric::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
  bool handled = false;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "NODE_REPORT") {
      handled = HandleNewNodeReport(msg);
      if(!handled)
        cout << "Unhandled Node Report From " << msg.GetCommunity() << endl;
    }

    if(msg.GetKey() == "REMOVE_NODE") {
      string node = msg.GetString();
//      cout << "----------------------- REMOVE NODE! ---------------------------" << endl;
//      cout << node << endl;
//      for (map<string, NodeRecord>::iterator it = m_map_node_records.begin(); it != m_map_node_records.end(); ++it) {
//        cout << (*it).first << endl;
//      }
      m_map_node_records.erase(node);
      m_ignored_nodes.push_back(node);
    }

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
   }

   return(true);
}

//---------------------------------------------------------
// Procedure: HandleNewNodeReport

bool FormationQualityMetric::HandleNewNodeReport(CMOOSMsg msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();

  for (unsigned int i=0; i<m_ignored_nodes.size(); i++) {
    if (community_name == m_ignored_nodes[i]) return true;
  }

  // update our auv node position
  m_map_node_records[community_name] = string2NodeRecord(message_value);

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool FormationQualityMetric::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool FormationQualityMetric::Iterate()
{
  m_iterations++;

  m_curr_node_offsets = mat(m_map_node_records.size(), 2);
  unsigned int i = 0;
  for (map<string, NodeRecord>::iterator it = m_map_node_records.begin(); it != m_map_node_records.end(); ++it) {
    m_curr_node_offsets(i, 0) = m_map_node_records[(*it).first].getX();
    m_curr_node_offsets(i, 1) = m_map_node_records[(*it).first].getY();
    i++;
  }

  if (m_curr_node_offsets.n_rows > 0) {
    assignmentScores(10, m_min_assignment);

    mat rotation;
    mat translation;
    mat solution;
    vec weights = ones<vec>(m_optimal_metric.n_rows);
    mat centroid_optimal_metric = (trans(weights)*m_optimal_metric)/accu(weights);
    mat centroid_curr_node_offsets = (trans(weights)*m_curr_node_offsets)/accu(weights);
    rigidTransformation2D(m_optimal_metric, m_curr_node_offsets, centroid_optimal_metric, centroid_curr_node_offsets, rotation, translation, solution);
    centroid_optimal_metric = (trans(weights)*solution)/accu(weights);

  //  mat solution_no_translation = trans(rotation*trans(m_optimal_metric));
    metricScore(solution, m_curr_node_offsets);

    m_num_nodes = m_optimal_metric.n_rows;

    if (m_display_rigid_registration) {
      for (unsigned int i = 0; i < solution.n_rows; i++) {
        m_warning_message << "fqm_" << i;
        m_abs_target.set_vertex(solution(i,0), solution(i,1));
        m_abs_target.set_color("vertex", "white");
        m_abs_target.set_label(m_warning_message.str());
        Notify("VIEW_POINT", m_abs_target.get_spec());
        m_warning_message.clear();
        m_warning_message.str("");
      }
    }

    if (m_display_rigid_registration && (m_num_nodes < m_num_nodes_prev)) {
      // remove previously drawn points that are no longer valid
      int diff = m_num_nodes_prev - m_num_nodes;
      for (unsigned int i = 1; i <= diff; i++) {
        m_warning_message << "fqm_" << m_num_nodes + i;
        m_abs_target.set_vertex(0, 0);
        m_abs_target.set_color("vertex", "invisible");
        m_abs_target.set_label(m_warning_message.str());
        Notify("VIEW_POINT", m_abs_target.get_spec());
        m_warning_message.clear();
        m_warning_message.str("");
      }
    }

    m_num_nodes_prev = m_num_nodes;
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool FormationQualityMetric::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);

      if (param == "NODE_OFFSETS_METRIC") {
        double offset_x, offset_y;
        bool result = MOOSValFromString(offset_x, value, "x");
        result = result && MOOSValFromString(offset_y, value, "y");
        m_vector_node_offsets.push_back(XYPoint(offset_x, offset_y));
        if (!result) return(false);
      }
    }
  }

  if (!m_MissionReader.GetConfigurationParam("DISPLAY_RIGID_REGISTRATION", m_display_rigid_registration)) {
    cerr << "Boolean DISPLAY_RIGID_REGISTRATION not specified! Quitting..." << endl;
    return(false);
  }

  m_node_offsets = mat(m_vector_node_offsets.size(), 2);
  for (unsigned int i = 0; i < m_vector_node_offsets.size(); i++) {
    m_node_offsets(i, 0) = m_vector_node_offsets[i].x();
    m_node_offsets(i, 1) = m_vector_node_offsets[i].y();
  }
  m_centroid_node_offsets = (ones(1, m_node_offsets.n_rows)*m_node_offsets)/m_node_offsets.n_rows;
  //find point in formation closest to centroid
  double min_dist = std::numeric_limits<double>::max();
  double min_dist_idx;
  for (unsigned int i = 0; i < m_node_offsets.n_rows; i++) {
    double curr_dist = hypot(m_node_offsets(i,0) - m_centroid_node_offsets(0,0), m_node_offsets(i,1) - m_centroid_node_offsets(0,1));
    if (curr_dist < min_dist) {
      min_dist = curr_dist;
      min_dist_idx = i;
    }
  }
  m_centroid_node_offsets(0,0) = m_node_offsets(min_dist_idx,0);
  m_centroid_node_offsets(0,1) = m_node_offsets(min_dist_idx,1);
  m_node_offsets = m_node_offsets - repmat(m_centroid_node_offsets, m_node_offsets.n_rows, 1);

  m_abs_target.set_color("label", "invisible");
  m_abs_target.set_color("vertex", "white");
  m_abs_target.set_vertex_size(10);

  m_num_nodes = 0;
  m_num_nodes_prev = 0;

  m_metric_score = std::numeric_limits<double>::max();

  m_timewarp = GetMOOSTimeWarp();

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void FormationQualityMetric::RegisterVariables()
{
  // m_Comms.Register("FOOBAR", 0);
  m_Comms.Register("NODE_REPORT", 0.0);                       // use node reports to get x/y of each node, instead of passing each individually
  m_Comms.Register("REMOVE_NODE", 0.0);
}

//---------------------------------------------------------------
// Procedure: rigidTransformation2D()
//   Purpose: Calculate the metric score (average Euclidean distance).
void FormationQualityMetric::metricScore(mat &opt_met, mat &curr_pos)
{
  cout << "OPTIMAL" << endl;
  cout << opt_met << endl;
  cout << "CURRENT VEH POS" << endl;
  cout << curr_pos << endl;
  mat diff = curr_pos - opt_met;
  cout << "DIFFERENCE" << endl;
  cout << diff << endl;
  double total_dist = 0;
  for (unsigned int i = 0; i < diff.n_rows; i++) {
    total_dist += hypot(diff(i,0), diff(i,1));
  }
  cout << "TOTAL DIST: " << total_dist << "\t -==- \t";
  m_metric_score = total_dist/opt_met.n_rows;
  cout << "AVG DIST: " << m_metric_score << endl << endl;
  Notify("METRIC_TOTAL_DIST", total_dist);
  Notify("METRIC_MEAN_DIST", m_metric_score);
}

//---------------------------------------------------------------
// Procedure: rigidTransformation2D()
//   Purpose: Perform calculations for rigid transformation.

void FormationQualityMetric::rigidTransformation2D(mat p, mat q, mat cent_p, mat cent_q, mat &R, mat &T, mat &sol)
{
  // Construct centered matrices
  mat x = p - repmat(cent_p, p.n_rows, 1);
  mat y = q - repmat(cent_q, q.n_rows, 1);

  // Construct covariance matrix
  mat H  = trans(x)*y;

  // Perform singular value decomposition
  mat U;
  vec s;
  mat V;
  svd(U, s, V, H);

  // Calculate rotation
  mat eyed;
  eyed.eye(H.n_rows, H.n_rows);
  eyed(H.n_rows-1, H.n_rows-1) = det(V*trans(U));
  R = V*eyed*trans(U);

  // Calculate translation
  T = trans(cent_q) - (R*trans(cent_p));

  // Perform transformation
  sol = R*trans(p) + repmat(T, 1, p.n_rows);
  sol = trans(sol);
}

//---------------------------------------------------------------
// Procedure: assignmentScores()
//   Purpose: Look at all node report positions, and all formation positions, and calculate the Euclidean distance score matrix for a number of angles

void FormationQualityMetric::assignmentScores(int angle_delta, umat &min_assignment)
{
  // Number of rotations and delta change in angle
  int num_rot = (int)(360.0/angle_delta);
  mat rot = mat(2,2);
  double sin_th = sin(angle_delta*M_PI/180.0);
  double cos_th = cos(angle_delta*M_PI/180.0);
  rot(0,0) = cos_th;
  rot(0,1) = -sin_th;
  rot(1,0) = sin_th;
  rot(1,1) = cos_th;

  m_centroid_curr_node_offsets = (ones(1, m_curr_node_offsets.n_rows)*m_curr_node_offsets)/m_curr_node_offsets.n_rows;
  //find point in formation closest to centroid
  double min_dist = std::numeric_limits<double>::max();
  double min_dist_idx;
  for (unsigned int i = 0; i < m_curr_node_offsets.n_rows; i++) {
    double curr_dist = hypot(m_curr_node_offsets(i,0) - m_centroid_curr_node_offsets(0,0), m_curr_node_offsets(i,1) - m_centroid_curr_node_offsets(0,1));
    if (curr_dist < min_dist) {
      min_dist = curr_dist;
      min_dist_idx = i;
    }
  }
  m_centroid_curr_node_offsets(0,0) = m_curr_node_offsets(min_dist_idx,0);
  m_centroid_curr_node_offsets(0,1) = m_curr_node_offsets(min_dist_idx,1);
  mat new_curr_node_offsets = m_curr_node_offsets - repmat(m_centroid_curr_node_offsets, m_curr_node_offsets.n_rows, 1);

  int rows = m_node_offsets.n_rows;
  int cols = new_curr_node_offsets.n_rows;
  mat costs = mat(rows, cols);

  if (rows > cols) {
    costs.insert_cols(new_curr_node_offsets.n_rows, rows - cols);
  }

  double min_cost = std::numeric_limits<double>::max();
  double curr_cost;
  int min_angle = 0;
  min_assignment = umat(rows, cols);
  umat curr_assignment(rows, cols);
  mat min_costs;
  for (unsigned int i = 0; i < num_rot; i++) {
    new_curr_node_offsets = trans(rot*trans(new_curr_node_offsets));
    for (unsigned int k = 0; k < new_curr_node_offsets.n_rows; k++) {
      for (unsigned int l = 0; l < m_node_offsets.n_rows; l++) {
        costs(l,k) = (hypot(new_curr_node_offsets(k,0) - m_node_offsets(l,0), new_curr_node_offsets(k,1) - m_node_offsets(l,1)));
      }
    }
    curr_assignment = hungarian(costs);
    curr_cost = accu(curr_assignment%costs);
    if (curr_cost < min_cost) {
      min_cost = curr_cost;
      min_assignment = curr_assignment;
      min_angle = (i+1)*angle_delta;
      min_costs = costs;
    }
  }

  // Get optimal formaion as subset of total formation
  m_optimal_metric = mat(new_curr_node_offsets.n_rows, new_curr_node_offsets.n_cols);
  umat indexes = linspace<umat>(0, min_assignment.n_rows-1, min_assignment.n_rows);
  indexes = trans(min_assignment)*indexes;
  for (unsigned int i = 0; i < m_optimal_metric.n_rows; i++) {
    m_optimal_metric(i, 0) = m_node_offsets(indexes(i), 0);
    m_optimal_metric(i, 1) = m_node_offsets(indexes(i), 1);
  }
}

/**MUNKRES ASSIGNMENT ALGORITHM START**/ //Lars Simon Zehnder - http://gallery.rcpp.org/articles/minimal-assignment/
void FormationQualityMetric::step_one(unsigned int &step, arma::mat &cost,
        const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        cost.row(r) -= arma::min(cost.row(r));
    }
    step = 2;
}

void FormationQualityMetric::step_two (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (cost.at(r, c) == 0.0 && rcov.at(r) == 0 && ccov.at(c) == 0) {
                indM.at(r, c)  = 1;
                rcov.at(r)     = 1;
                ccov.at(c)     = 1;
                break;                                              // Only take the first
                                                                    // zero in a row and column
            }
        }
    }
    /* for later reuse */
    rcov.fill(0);
    ccov.fill(0);
    step = 3;
}

void FormationQualityMetric::step_three(unsigned int &step, const arma::umat &indM,
        arma::ivec &ccov, const unsigned int &N)
{
    unsigned int colcount = 0;
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (indM.at(r, c) == 1) {
                ccov.at(c) = 1;
            }
        }
    }
    for (unsigned int c = 0; c < N; ++c) {
        if (ccov.at(c) == 1) {
            ++colcount;
        }
    }
    if (colcount == N) {
        step = 7;
    } else {
        step = 4;
    }
}

void FormationQualityMetric::find_noncovered_zero(int &row, int &col,
        const arma::mat &cost, const arma::ivec &rcov,
        const arma::ivec &ccov, const unsigned int &N)
{
    unsigned int r = 0;
    unsigned int c;
    bool done = false;
    row = -1;
    col = -1;
    while (!done) {
        c = 0;
        while (true) {
            if (cost.at(r, c) == 0.0 && rcov.at(r) == 0 && ccov.at(c) == 0) {
                row = r;
                col = c;
                done = true;
            }
            ++c;
            if (c == N || done) {
                break;
            }
        }
        ++r;
        if (r == N) {
            done = true;
        }
    }
}

bool FormationQualityMetric::star_in_row(int &row, const arma::umat &indM,
        const unsigned int &N)
{
    bool tmp = false;
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 1) {
            tmp = true;
            break;
        }
    }
    return tmp;
}

void FormationQualityMetric::find_star_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N)
{
    col = -1;
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 1) {
            col = c;
        }
    }
}

void FormationQualityMetric::step_four (unsigned int &step, const arma::mat &cost,
        arma::umat &indM, arma::ivec &rcov, arma::ivec &ccov,
        int &rpath_0, int &cpath_0, const unsigned int &N)
{
    int row = -1;
    int col = -1;
    bool done = false;
    while(!done) {
        find_noncovered_zero(row, col, cost, rcov,
                ccov, N);

        if (row == -1) {
            done = true;
            step = 6;
        } else {
            /* uncovered zero */
            indM(row, col) = 2;
            if (star_in_row(row, indM, N)) {
                find_star_in_row(row, col, indM, N);
                /* Cover the row with the starred zero
                 * and uncover the column with the starred
                 * zero.
                 */
                rcov.at(row) = 1;
                ccov.at(col) = 0;
            } else {
                /* No starred zero in row with
                 * uncovered zero
                 */
                done = true;
                step = 5;
                rpath_0 = row;
                cpath_0 = col;
            }
        }
    }
}

void FormationQualityMetric::find_star_in_col (const int &col, int &row,
        const arma::umat &indM, const unsigned int &N)
{
    row = -1;
    for (unsigned int r = 0; r < N; ++r) {
        if (indM.at(r, col) == 1) {
            row = r;
        }
    }
}

void FormationQualityMetric::find_prime_in_row (const int &row, int &col,
        const arma::umat &indM, const unsigned int &N)
{
    for (unsigned int c = 0; c < N; ++c) {
        if (indM.at(row, c) == 2) {
            col = c;
        }
    }
}

void FormationQualityMetric::augment_path (const int &path_count, arma::umat &indM,
        const arma::imat &path)
{
    for (unsigned int p = 0; p < path_count; ++p) {
        if (indM.at(path(p, 0), path(p, 1)) == 1) {
            indM.at(path(p, 0), path(p, 1)) = 0;
        } else {
            indM.at(path(p, 0), path(p, 1)) = 1;
        }
    }
}

void FormationQualityMetric::clear_covers (arma::ivec &rcov, arma::ivec &ccov)
{
    rcov.fill(0);
    ccov.fill(0);
}

void FormationQualityMetric::erase_primes(arma::umat &indM, const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (indM.at(r, c) == 2) {
                indM.at(r, c) = 0;
            }
        }
    }
}

void FormationQualityMetric::step_five (unsigned int &step,
        arma::umat &indM, arma::ivec &rcov,
        arma::ivec &ccov, arma::imat &path,
        int &rpath_0, int &cpath_0,
        const unsigned int &N)
{
    bool done = false;
    int row = -1;
    int col = -1;
    unsigned int path_count = 1;
    path.at(path_count - 1, 0) = rpath_0;
    path.at(path_count - 1, 1) = cpath_0;
    while (!done) {
        find_star_in_col(path.at(path_count - 1, 1), row,
                indM, N);
        if (row > -1) {
            /* Starred zero in row 'row' */
            ++path_count;
            path.at(path_count - 1, 0) = row;
            path.at(path_count - 1, 1) = path.at(path_count - 2, 1);
        } else {
            done = true;
        }
        if (!done) {
            /* If there is a starred zero find a primed
             * zero in this row; write index to 'col' */
            find_prime_in_row(path.at(path_count - 1, 0), col,
                    indM, N);
            ++path_count;
            path.at(path_count - 1, 0) = path.at(path_count - 2, 0);
            path.at(path_count - 1, 1) = col;
        }
    }
    augment_path(path_count, indM, path);
    clear_covers(rcov, ccov);
    erase_primes(indM, N);
    step = 3;
}

void FormationQualityMetric::find_smallest (double &minval, const arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N)
{
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (rcov.at(r) == 0 && ccov.at(c) == 0) {
                if (minval > cost.at(r, c)) {
                    minval = cost.at(r, c);
                }
            }
        }
    }
}

void FormationQualityMetric::step_six (unsigned int &step, arma::mat &cost,
        const arma::ivec &rcov, const arma::ivec &ccov,
        const unsigned int &N)
{
    double minval = std::numeric_limits<double>::max();;
    find_smallest(minval, cost, rcov, ccov, N);
    for (unsigned int r = 0; r < N; ++r) {
        for (unsigned int c = 0; c < N; ++c) {
            if (rcov.at(r) == 1) {
                cost.at(r, c) += minval;
            }
            if (ccov.at(c) == 0) {
                cost.at(r, c) -= minval;
            }
        }
    }
    step = 4;
}

arma::umat FormationQualityMetric::hungarian(const arma::mat &input_cost)
{
    const unsigned int N = input_cost.n_rows;
    unsigned int step = 1;
    int cpath_0 = 0;
    int rpath_0 = 0;
    arma::mat cost(input_cost);
    arma::umat indM(N, N);
    arma::ivec rcov(N);
    arma::ivec ccov(N);
    arma::imat path(2 * N, 2);

    indM = arma::zeros<arma::umat>(N, N);
    bool done = false;
    while (!done) {
        switch (step) {
            case 1:
                step_one(step, cost, N);
                break;
            case 2:
                step_two(step, cost, indM, rcov, ccov, N);
                break;
            case 3:
                step_three(step, indM, ccov, N);
                break;
            case 4:
                step_four(step, cost, indM, rcov, ccov,
                        rpath_0, cpath_0, N);
                break;
            case 5:
                step_five(step, indM, rcov, ccov,
                        path, rpath_0, cpath_0, N);
                break;
            case 6:
                step_six(step, cost, rcov, ccov, N);
                break;
            case 7:
                done = true;
                break;
        }
    }
    return indM;
}
/**MUNKRES ASSIGNMENT ALGORITHM END**/
