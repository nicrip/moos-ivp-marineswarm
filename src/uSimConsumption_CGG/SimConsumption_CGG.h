/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimConsumption_CGG.h                             */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#ifndef SimConsumption_HEADER
#define SimConsumption_HEADER

#include <map>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"

class SimConsumption : public AppCastingMOOSApp
{
  public:
    SimConsumption();
    ~SimConsumption();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();
    bool HandleNewPing(CMOOSMsg msg);
    double GetGlobalConsumption();
    void HandlePowerSpeed(std::string value); // to use with uSimMarine

  private:
    // Configuration
    double m_efficiency_prop;
    double m_efficiency_shaft;
    double m_efficiency_gear;
    double m_efficiency_motor;
    double m_power_nominal;
    double m_consumption_per_ping;
    double m_energy_pack;
    bool m_no_acceleration_no_deceleration;

    // State
    double m_power_prop;
    double m_conso_prop;
    double m_conso_comm;
    double m_conso_nominal;
    double m_pings_number;
    double m_previous_iteration_date;
    double m_db_uptime;
    double m_heading_thrust;
    bool   m_energy_pack_expended;
    double m_final_duration;

    // Input
    double m_speed;     // to use with uSimMarine
    double m_drag_coef; // to use with uSimMarine // drag = m_drag_coef * speed * fabs(speed)
    double m_thrust_force;
    double m_surge_speed;
};

#endif
