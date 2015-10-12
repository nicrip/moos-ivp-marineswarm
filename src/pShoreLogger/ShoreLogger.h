/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: ShoreLogger.h                                   */
/*    DATE:                                                 */
/************************************************************/

#ifndef ShoreLogger_HEADER
#define ShoreLogger_HEADER

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class ShoreLogger : public AppCastingMOOSApp
{
 public:
   ShoreLogger();
   ~ShoreLogger();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool buildReport();

 private: // Configuration variables
   bool         m_log_shore;
   double       m_log_time_delta;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   //variables from formation metric
   double       m_metric_total_dist;
   double       m_metric_mean_dist;
   //other variables
   double       m_start_MOOSTime;
   double       m_previous_MOOSTime;
   bool         m_first_log;
   double       m_deploy;
};

#endif
