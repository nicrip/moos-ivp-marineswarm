/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimDeployment_CGG.h                              */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#ifndef SimDeployment_HEADER
#define SimDeployment_HEADER

#include <map>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class SimDeployment : public AppCastingMOOSApp
{
  public:
    SimDeployment();
    ~SimDeployment();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();
    
  private:
    // <id_waypoint, <auv_names> >
    std::map<int, std::vector<std::string> > m_map_launches;
    double m_nav_x, m_nav_y;
    int m_current_launch_point;
};

#endif 
