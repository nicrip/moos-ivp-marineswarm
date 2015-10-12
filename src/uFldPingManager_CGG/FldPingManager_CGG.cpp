/* ******************************************************** */
/*    NAME: Simon Rohou, modified by Mohamed Saad Ibn Seddik*/
/*      to support AppCasting                               */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: FldPingManager_CGG.h                            */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#include <math.h>
#include <iterator>
#include <algorithm>
#include "MBUtils.h"
#include "AngleUtils.h"
#include "FldPingManager_CGG.h"
#include "CoordinateSystem/SphericalCoordinateSystem.h"
#include "NodeRecordUtils.h"
#include "XYCommsPulse.h"
#include "ColorParse.h"

using namespace std;
int FldPingManager::sensors_number_;

FldPingManager* FldPingManager::inst_ = 0;
boost::shared_ptr<FldPingManagerConfig> global_cfg_;



FldPingManager* FldPingManager::get_instance()
{
    if(!inst_)    {
        global_cfg_.reset(new FldPingManagerConfig);
        inst_ = new FldPingManager(*global_cfg_);
    }
    return inst_;
}

//---------------------------------------------------------
// Constructor

FldPingManager::FldPingManager(FldPingManagerConfig& cfg)
    : GobyMOOSAppSelector<AppCastingMOOSApp>(&cfg),
      cfg_(cfg)
{
  m_colors.push_back("red");
  m_colors.push_back("blue");
  m_colors.push_back("green");
  m_colors.push_back("yellow");
  m_colors.push_back("purple");
  m_colors.push_back("orange");
  m_colors.push_back("magenta");
  m_colors.push_back("brown");
  m_colors.push_back("cyan");
  m_colors.push_back("pink");
  
  FldPingManager::sensors_number_ = 0;
  
  m_n_negative_dt = 0;
  m_n_outputs = 0;
  m_ping_id = 0;

  m_launching_date = MOOSTime();

  subscribe("PING", &FldPingManager::HandleNewPing, this);   
  subscribe("NODE_REPORT", &FldPingManager::HandleNewNodeReport, this);    

}

//---------------------------------------------------------
// Destructor

FldPingManager::~FldPingManager()
{
  
}

void FldPingManager::loop()
{
  // analysing each ping scheduled
  
  // each receiver
  map<string,vector<PingRecord> >::iterator receiver_pings = m_map_pings_to_be_transmitted.begin();
  while(receiver_pings != m_map_pings_to_be_transmitted.end())
  {
    string receiver_name = receiver_pings->first;
    
    // each ping for this receiver
    vector<PingRecord>::iterator ping_to_receive = receiver_pings->second.begin();
    while(ping_to_receive != receiver_pings->second.end())
    {
      double current_time = MOOSTime();
      double medium_depth = (ping_to_receive->getStartDepth() + m_map_node_records[receiver_name].getDepth()) / 2.;
      double sound_speed = GetSoundSpeed();
      
      // we need to compare two dates to know the end of life of the considered ping:
      double date__ping_is_beyond_range = ping_to_receive->getStartingTime() + (ping_to_receive->getRange() / sound_speed);
      double date__ping_reaches_receiver; // for this date, we need to compute a linear interpolation
      
      double current_rho, current_theta, current_phi;
      GetCurrentGap(*ping_to_receive, m_map_node_records[receiver_name], 
                    sound_speed, current_time,
                    current_rho, current_theta, current_phi);
      
      // linear interpolation between two iteration date:
      // we study the gap between the receiver and the ping
      // if the gap is negative, we can consider the ping is received at 't' date
      
      // the previous iteration date
      double t0 = ping_to_receive->getPreviousDate(); 
      // the current date
      double t1 = current_time; 
      // the previous iteration gap
      double g0 = ping_to_receive->getPreviousGapWithReceiver(); 
      // the current gap
      double g1 = current_rho; 
      
      bool simulate_ping_absorption, simulate_ping_reception;
      
      // linear interpolation:
      if(g1 != g0)
        date__ping_reaches_receiver = ((g0 * t1) - (t0 * g1)) / (g0 - g1);
      
      else
        date__ping_reaches_receiver = t1 + 1000.; // a hypothetical future date
      
      bool range_reached_since_last_it = (date__ping_is_beyond_range <= t1);
      bool ping_received_since_last_it = (date__ping_reaches_receiver <= t1);
      bool range_reached_before_ping = (date__ping_is_beyond_range < date__ping_reaches_receiver);
      
      // the ping is absorbed by water:
      //    - if the range has been reached since the last iteration
      //      AND
      //        - if the range has been reached before the ping is received
      //          OR
      //        - if the ping has not been received since the last iteration
      simulate_ping_absorption = range_reached_since_last_it && 
                        (range_reached_before_ping || !ping_received_since_last_it);
      
      // the ping is received by the vehicle:
      //    - if the ping has been received since the last iteration
      //      AND
      //        - if the ping has been received before the range is reached
      //          OR
      //        - if the range has not been reached since the last iteration
      simulate_ping_reception = ping_received_since_last_it && 
                        (!range_reached_before_ping || !range_reached_since_last_it);
      
      // note: simulate_ping_absorption and simulate_ping_reception can't be true at the same iteration
      
      if(simulate_ping_absorption)
      {
        // sound absorbed by water: the ping_to_receive is removed
        ping_to_receive = (receiver_pings->second).erase(ping_to_receive);
      }
      
      else if(simulate_ping_reception)
      {
        // the ping reception is simulated (with noise)
        double dt_noised = max(0.5, GetNoisyRho((date__ping_reaches_receiver - ping_to_receive->getStartingTime()) * sound_speed)) / sound_speed;
        double theta_noised = GetNoisyTheta(current_theta);
        double phi_noised = GetNoisyPhi(current_phi);
        SimulatePingReception(*ping_to_receive, receiver_name, dt_noised, theta_noised, phi_noised);
        
        if(cfg_.display_ping_reception())
        {
          // the ping reception is displayed
          DisplayPingReception(*ping_to_receive, m_map_node_records[receiver_name]);
        }
        
        // the ping is received once
        ping_to_receive = (receiver_pings->second).erase(ping_to_receive);
      }
      
      else
      {
        ping_to_receive->setPreviousDate(current_time);
        ping_to_receive->setPreviousGapWithReceiver(current_rho);
        ++ping_to_receive;
      }
    }
    
    ++receiver_pings;
  }

  AppCastingMOOSApp::PostReport();
}

//---------------------------------------------------------
// Procedure: SimulatePingReception

void FldPingManager::SimulatePingReception(const PingRecord& ping, string receiver_name, double dt, double theta, double phi)
{
    string sender_name = ping.getSenderName();
    double sender_heading = ping.getSenderHeading();
    string data = ping.getData();
    int modem_id = ping.getModemId();

    ostringstream strs;
  
    if(dt < 0)
        m_n_negative_dt ++;
  
    m_ping_id ++;
  
    strs << "dt=" << dt <<
        ",theta=" << degToRadians(angle360(radToDegrees(theta))) <<
        ",phi=" << degToRadians(angle360(radToDegrees(phi))) <<
        ",name=" << sender_name <<
        ",sender_heading=" << sender_heading <<
        ",ping_id=" << m_ping_id <<
        ",modem_id=" << modem_id;
    if(!data.empty())
        strs << ",data=" << data;

    Notify("PING_REPORT_FOR_" + toupper(receiver_name), strs.str());
    m_n_outputs ++;
}

//---------------------------------------------------------
// Procedure: HandleNewNodeReport

void FldPingManager::HandleNewNodeReport(const CMOOSMsg& msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();
  m_map_node_records[community_name] = string2NodeRecord(message_value);
  
  // if the node has just been discovered (= is not associated with a color)
  if(m_map_node_records_color.find(community_name) == m_map_node_records_color.end())
  {
    // an empty ping report is sent to the vehicle
    Notify("PING_REPORT_FOR_" + toupper(community_name), "discovering");
    
    // each sensor's ping is displayed with a specific color
    m_map_node_records_color[community_name] = m_colors[FldPingManager::sensors_number_ % m_colors.size()];
    FldPingManager::sensors_number_ ++;
  }
  
}

//---------------------------------------------------------
// Procedure: HandleNewPing

void FldPingManager::HandleNewPing(const CMOOSMsg& msg)
{
  string ping_value = msg.GetString();
  string community_name = msg.GetCommunity();
  
  // if the sender node has been discovered
  if(m_map_node_records.find(community_name) != m_map_node_records.end())
  {
    // sending a ping is scheduled for other vehicles
    PingRecord new_ping;
    double range;
    string color;
    MOOSValFromString(range, ping_value, "range");

    std::string data;
    if(MOOSValFromString(data, ping_value, "data"))
        new_ping.setData(data);
    int id;
    if(MOOSValFromString(id, ping_value, "modem_id"))
        new_ping.setModemId(id);
     
    new_ping.setStartPosX(m_map_node_records[community_name].getX());
    new_ping.setStartPosY(m_map_node_records[community_name].getY());
    new_ping.setStartDepth(m_map_node_records[community_name].getDepth());
    new_ping.setStartingTime(msg.GetTime());
    new_ping.setSenderName(community_name);
    new_ping.setSenderHeading(degToRadians(m_map_node_records[community_name].getHeading()));
    new_ping.setPreviousDate(msg.GetTime());
    
    if(cfg_.display_ping_emission())
      DisplayPingEmission(new_ping);
    
    // all vehicles are candidates for receipt
    for(map<string,NodeRecord>::iterator potential_receiver = m_map_node_records.begin() ; 
        potential_receiver != m_map_node_records.end() ; 
          potential_receiver++)
    {
      if(potential_receiver->first != community_name)
      {
        double rho, theta, phi;
        GetCurrentGap(new_ping, potential_receiver->second, 
                      GetSoundSpeed(), new_ping.getStartingTime(),
                      rho, theta, phi);
        new_ping.setPreviousGapWithReceiver(rho);
        new_ping.setRange(GetRandomRange(range)); // the sound propagation may not have the same range in all directions
        m_map_pings_to_be_transmitted[potential_receiver->first].push_back(new_ping);
      }
    }
  }
  
}

//---------------------------------------------------------
// Procedure: GetRandomRange

double FldPingManager::GetRandomRange(double acoustic_range)
{
  if(cfg_.exponentially_decaying_detection())
  {
    // generate a random double in the range [0,1]
    int rand_int = rand() % 10000;
    double rand_pct = ((double)(rand_int) / 10000);
    return acoustic_range + 200. * exp(rand_pct * 10 * log(1./2.));
  }

  else
    return acoustic_range;
}

//---------------------------------------------------------
// Procedure: DisplayPingReception

void FldPingManager::DisplayPingReception(PingRecord ping_to_receive, NodeRecord receiver_node)
{
  if(receiver_node.getName() != "mothership")
  {
    XYCommsPulse pulse(ping_to_receive.getStartPosX(), 
                        ping_to_receive.getStartPosY(), 
                        receiver_node.getX(), 
                        receiver_node.getY());
    
    pulse.set_duration(3.5);
    pulse.set_label(ping_to_receive.getSenderName() + "2" + receiver_node.getName());
    pulse.set_time(MOOSTime());
    pulse.set_beam_width(7.);
    pulse.set_fill(0.35);
    pulse.set_color("fill", m_map_node_records_color[receiver_node.getName()]);
    string pulse_spec = pulse.get_spec();
    Notify("VIEW_COMMS_PULSE", pulse_spec);
  }
}

//---------------------------------------------------------
// Procedure: BuildReport

bool FldPingManager::buildReport()
{
  m_msgs << endl;
  m_msgs << "uFldPingManager configured parameters:" << endl;
  m_msgs << "  - SOUND_SPEED_MIN = " << cfg_.sound_speed_min() << " m/s" << endl;
  m_msgs << "  - SOUND_SPEED_MAX = " << cfg_.sound_speed_max() << " m/s" << endl;
  m_msgs << "  - SOUND_SPEED_variation = " << setprecision(7) << cfg_.sound_speed_variation() << endl;
  m_msgs << "     - current sound speed = " << GetSoundSpeed() << "m/s" << endl;
  m_msgs << "  - EXPONENTIALLY_DECAYING_DETECTION = " << cfg_.exponentially_decaying_detection() << endl;

  m_msgs << "\nuFldPingManager discovered " << m_map_node_records.size() << " Ping sensor." << endl;

  m_msgs << "\nNoise parameters:";
  
  m_msgs << endl << "  - rho noised: ";
  
  if(cfg_.rho_noised())
    m_msgs << "sigma = " << cfg_.rho_gaussian_sigma() << "m";
  
  else
    m_msgs << "none";
  
  m_msgs << endl << "  - theta noised: ";
  
  if(cfg_.theta_noised())
    m_msgs << "sigma = " << cfg_.theta_gaussian_sigma() << "°";
  
  else
    m_msgs << "none";
  
  m_msgs << endl << "  - phi noised: ";
  
  if(cfg_.phi_noised())
    m_msgs << "sigma = " << cfg_.phi_gaussian_sigma() << "°";
  
  else
    m_msgs << "none";

  m_msgs << "\n\nNegative dt: " << m_n_negative_dt << endl;
  m_msgs << "\n\nOutputs: " << m_n_outputs;

  return true;
}

//------------------------------------------------------------
// Procedure: GetNoisyRho()

double FldPingManager::GetNoisyRho(double true_distance) const
{
  double noise = 0.;
  
  if(cfg_.rho_noised())
    noise = MOOSWhiteNoise(cfg_.rho_gaussian_sigma());
    
  return true_distance + noise;
}

//------------------------------------------------------------
// Procedure: GetNoisyTheta()

double FldPingManager::GetNoisyTheta(double true_angle) const
{
  double noise = 0.;
  
  if(cfg_.theta_noised())
    noise = degToRadians(MOOSWhiteNoise(cfg_.theta_gaussian_sigma()));
    
  return true_angle + noise;
}

//------------------------------------------------------------
// Procedure: GetNoisyPhi()

double FldPingManager::GetNoisyPhi(double true_angle) const
{
  double noise = 0.;
  
  if(cfg_.phi_noised())
    noise = degToRadians(MOOSWhiteNoise(cfg_.phi_gaussian_sigma()));
    
  return true_angle + noise;
}

//------------------------------------------------------------
// Procedure: GetSoundSpeed()

double FldPingManager::GetSoundSpeed()
{
  if(cfg_.sound_speed_variation())
      return cfg_.sound_speed_min() + ((MOOSTime() - m_launching_date) * (cfg_.sound_speed_max() - cfg_.sound_speed_min()) / (cfg_.mission_duration() * 24 * 3600));

  else
    return (cfg_.sound_speed_min() + cfg_.sound_speed_max()) / 2.;
}

//------------------------------------------------------------
// Procedure: GetCurrentGap()

void FldPingManager::GetCurrentGap(PingRecord ping_to_receive, NodeRecord receiver, 
                  double sound_speed, double current_time,
                  double& rho, double& theta, double& phi)
{
  double x1 = ping_to_receive.getStartPosX();
  double y1 = ping_to_receive.getStartPosY();
  double z1 = -1. * (ping_to_receive.getStartDepth()); // z and depth are opposed
  double x2 = receiver.getX();
  double y2 = receiver.getY();
  double z2 = -1. * (receiver.getDepth()); // z and depth are opposed
  
  // The receiver is placed at the origin of a spherical coordinate system
  SphericalCoordinateSystem::Cartesian2Spherical(x1 - x2, y1 - y2, z1 - z2, rho, theta, phi);
  
  // The distance propagation is subtracted
  rho -= sound_speed * (current_time - ping_to_receive.getStartingTime());
  
  // if rho < 0, the ping is beyond the receiver: the reception is done
}

//---------------------------------------------------------
// Procedure: DisplayPingEmission

void FldPingManager::DisplayPingEmission(PingRecord ping)
{
  // Display circles around emitters:
  std::ostringstream s;
  s << "x=" << ping.getStartPosX() 
    << ",y=" << ping.getStartPosY() 
    << ",radius=" << 30.0 
    << ",duration=" << 1.0 
    << ",fill=" << 0.9
    << ",label=" << "ping_" << ping.getSenderName() 
    << ",edge_color=" << "white"
    << ",fill_color=" << "white" 
    << ",time=" << setprecision(16) << ping.getStartingTime() 
    << ",edge_size=" << 1.0;
  Notify("VIEW_RANGE_PULSE", s.str());
}
