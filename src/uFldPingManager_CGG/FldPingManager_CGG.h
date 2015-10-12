/* ******************************************************** */
/*    NAME: Simon Rohou, modified by Mohamed Saad Ibn Seddik*/
/*      to support AppCasting                               */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: FldPingManager.h                                */
/*    DATE: february 2014                                   */
/*                                                          */
/*    Typically run in a shoreside community.               */
/*    The uFldPingManager application is used primarily in  */
/*    coordination with uFldPingSensor, running on the      */
/*    vehicles.                                             */
/*                                                          */
/*    When the application receive a virtual ping from a    */
/*    vehicle, it transmits the ping to its neighbors with  */
/*    a delay according to their distance. If the ping goes */
/*    beyond the acoustic range, it is absorbed by water    */
/*    with an exponentially decaying probability (optional).*/
/*                                                          */
/*    The sound speed (needed for distance estimation) is   */
/*    computed knowing the temperature, the salinity and    */
/*    the depth of the environment.                         */
/*                                                          */
/*    Variables Published by uFldPingSensor:                */
/*      -PING_REPORT_FOR_$V: the PingRecord symbolizing a   */
/*     ping reception for a vehicle nammed "V".             */
/*     It contains (dt, theta, phi, name) about the sender  */
/*      The position is expressed in a spherical coordinate */
/*     system                                               */
/*      -VIEW_COMMS_PULSE: a visual artifact for rendering  */
/*      the sending of a node report or node message        */
/*      between vehicles                                    */
/*                                                          */
/*    Variables Subscribed for uFldPingSensor:              */
/*      -PING: contains (x, y, depth, name) about the sender*/
/*      and the emission date                               */
/*      -NODE_REPORT: the MOOS NodeRecord symbolizing a     */
/*     vehicle                                              */
/* ******************************************************** */

#ifndef FldPingManager_CGG_HEADER
#define FldPingManager_CGG_HEADER

#include "goby/moos/goby_moos_app.h"
#include <map>
#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"
#include "NodeMessage.h"
#include "FldPingManager_CGG_Test.h"
#include "Ping/PingRecord.h"
#include "fld_ping_manager_config.pb.h"

class FldPingManager : public GobyMOOSAppSelector<AppCastingMOOSApp>
{
  public:
    static FldPingManager* get_instance();

  private:
    FldPingManager(FldPingManagerConfig& cfg);
    ~FldPingManager();

  protected:
    void loop();
    void HandleNewNodeReport(const CMOOSMsg& msg);
    void HandleNewPing(const CMOOSMsg& msg);
    void SimulatePingReception(const PingRecord& ping, std::string receiver_name, double dt, double theta, double phi);
    void GetCurrentGap(PingRecord ping_to_receive, NodeRecord receiver, double sound_speed, double current_time, double& rho, double& theta, double& phi);
    void DisplayPingReception(PingRecord ping_to_receive, NodeRecord receiver_node);
    void DisplayPingEmission(PingRecord ping);
    double GetNoisyRho(double true_distance) const;
    double GetNoisyTheta(double true_angle) const;
    double GetNoisyPhi(double true_angle) const;
    double GetRandomRange(double acoustic_range);
    double GetSoundSpeed();
    bool buildReport();
    
  private:
    double      m_timewarp;
    int         m_n_negative_dt;
    int         m_n_outputs;
    double      m_launching_date; // MOOSTime
    int         m_ping_id;
    
    static int  sensors_number_; // used for ping colors
    static FldPingManager* inst_;    
    FldPingManagerConfig& cfg_;
    
    // <color>
    std::vector<std::string> m_colors;
    // <vehicle_name,node_record>
    std::map<std::string,NodeRecord> m_map_node_records;
    // <vehicle_name,color>
    std::map<std::string,std::string> m_map_node_records_color;
    // <receiver_vehicle_name,<pings_to_be_transmitted>>
    std::map<std::string,std::vector<PingRecord> > m_map_pings_to_be_transmitted;
  
  // for unit tests:
  friend void FldPingManager_launchTestsAndExitIfOk();
};

#endif 
