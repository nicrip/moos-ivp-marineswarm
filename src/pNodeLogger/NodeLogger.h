/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: NodeLogger.h                                    */
/*    DATE:                                                 */
/************************************************************/

#ifndef NodeLogger_HEADER
#define NodeLogger_HEADER

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"

class NodeLogger : public AppCastingMOOSApp
{
 public:
   NodeLogger();
   ~NodeLogger();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool buildReport();

 private: // Configuration variables
   bool         m_log_node;
   double       m_log_time_delta;
   std::string  m_log_folder;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   //variables from controller
   double       m_nav_x;
   double       m_nav_y;
   double       m_nav_depth;
   double       m_nav_heading;
   double       m_nav_speed;
   double       m_previous_nav_x;
   double       m_previous_nav_y;
   //variables from current simulator
   double       m_drift_x;
   double       m_drift_y;
   //variables from consumption monitor
   double       m_conso_nominal;
   double       m_conso_prop;
   double       m_conso_comm;
   double       m_conso_all;
   double       m_power_nominal;
   double       m_power_prop;
   double       m_estimation_duration_days;
   //variables from drifting target behavior BHV_DriftingTarget
   double       m_time_drifting;
   double       m_time_thrusting;
   double       m_drifting_ratio;
   //other variables
   double       m_start_MOOSTime;
   double       m_previous_MOOSTime;
   bool         m_first_log;
   double       m_deploy;
   //distance_metrics
   double       m_drifting_distance;
   double       m_thrusting_distance;
   double       m_total_distance;
   double       m_dist_drifting_ratio;
};

#endif
