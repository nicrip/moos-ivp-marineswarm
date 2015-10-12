/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimShot_CGG.h                                    */
/*   DATE: june 2014                                        */
/* ******************************************************** */

#ifndef SimShot_HEADER
#define SimShot_HEADER

#include <map>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class SimShot : public AppCastingMOOSApp
{
  public:
    SimShot();
    ~SimShot();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();
    void DisplayShot();
    
  private:
    double m_nav_x, m_nav_y;
    bool m_deploy_shooters;
    bool m_order_next_shot;
    double m_shot_date;
    double m_time_between_shots;
    int m_shot_number;
    std::string m_next_shooter;
};

#endif 
