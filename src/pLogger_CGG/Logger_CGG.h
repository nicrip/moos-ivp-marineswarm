/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: Logger_CGG.h                                     */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#ifndef Logger_HEADER
#define Logger_HEADER

#include <map>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"

class Logger : public AppCastingMOOSApp
{
  public:
    Logger();
    ~Logger();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();
    
  private:
    double m_nav_x;
    double m_nav_y;
    double m_previous_nav_x;
    double m_previous_nav_y;
    double m_nav_depth;
    double m_active_speed;
    double m_drift_x;
    double m_drift_y;
    double m_drift_distance;
    double m_conso_nominal;
    double m_conso_prop;
    double m_conso_comm;
    double m_conso_all;
    double m_ratio_recording;
    double m_estimation_duration_days;
    double m_starting_time;
    double m_previous_log_date;
    double m_distance_over_ground;
    double m_active_distance;
    double m_log_time_gap;
    bool m_distance_over_ground_computable;
    bool m_enable_logging;
};

#endif 