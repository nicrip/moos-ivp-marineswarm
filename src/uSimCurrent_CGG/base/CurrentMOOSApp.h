/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: CurrentMOOSApp.h                                     */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#ifndef CURRENT_CGG_MOOSAPP_HEADER
#define CURRENT_CGG_MOOSAPP_HEADER

#include <map>
#include <string>
#include <utility>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"
#include "Current/CurrentModel.h"
#ifdef BUILD_GUI_CODE
  #include "../base_and_viewer/CurrentViewer.h"
#endif

class CurrentMOOSApp : public AppCastingMOOSApp
{
  public:
    CurrentMOOSApp();
    ~CurrentMOOSApp();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool HandleNewNodeReport(CMOOSMsg msg);
    void AddCurrentModel(std::string value);
    bool InitializeCurrentModels();
    bool buildReport();
    
  private:
    int m_updated_currents;
    bool m_display_viewer;
    double m_starting_time;
    std::map<std::string,NodeRecord> m_map_node_records; // <vehicle_name,node_record>
    std::map<std::string,std::pair<double,double> > m_map_node_hdrifts; // <vehicle_name,<drift_x,drift_y> >
    std::vector<CurrentModel*> m_current_models;
    
    #ifdef BUILD_GUI_CODE
      CurrentViewer* m_viewer;
    #endif
};

#endif 
