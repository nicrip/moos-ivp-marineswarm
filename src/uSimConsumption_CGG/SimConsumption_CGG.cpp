/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: SimConsumption_CGG.cpp                           */
/*   DATE: april 2014                                       */
/* ******************************************************** */

#include <math.h>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "MBUtils.h"
#include "SimConsumption_CGG.h"
#include "NodeRecordUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

SimConsumption::SimConsumption()
{
  m_conso_prop            = 0.;
  m_conso_comm            = 0.;
  m_conso_nominal         = 0.;
  m_thrust_force          = 0.;
  m_surge_speed           = 0.;
  m_speed                 = 0.; // to use with uSimMarine
  m_pings_number          = 0;

  m_efficiency_prop       = 0.5;
  m_efficiency_shaft      = 0.5;
  m_efficiency_gear       = 0.5;
  m_efficiency_motor      = 0.5;
  m_power_nominal         = 1.;
  m_consumption_per_ping  = 1.;

  m_no_acceleration_no_deceleration = false;
  m_energy_pack_expended = false;
}

//---------------------------------------------------------
// Destructor

SimConsumption::~SimConsumption()
{

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool SimConsumption::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    bool handled = false;

    if(msg.GetKey() == "PING")
      handled = HandleNewPing(msg);

    if(msg.GetKey() == "NAV_THRUST_FORCE")
      m_thrust_force = msg.GetDouble();

    if(msg.GetKey() == "NAV_SURGE_SPEED")
      m_surge_speed = msg.GetDouble();

    if(msg.GetKey() == "DB_UPTIME")
      m_db_uptime = msg.GetDouble();

    if(msg.GetKey() == "NAV_SPEED") // to use with uSimMarine
    {
      m_speed = msg.GetDouble();
      m_surge_speed = m_speed;
      m_thrust_force = m_drag_coef * m_speed * fabs(m_speed);
    }

    //PLACEHOLDER FOR PITCH, SURGE AND HEADING THRUSTS, FOR POWER CONSUMPTION WHEN TURNING!
    if(msg.GetKey() == "NAV_HEADING_THRUST")
    {
      m_heading_thrust = msg.GetDouble();
    }
    //---------------------------------------------

    if(!handled)
      cout << "Unhandled Mail: " << msg.GetKey() << endl;

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

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool SimConsumption::OnConnectToServer()
{
  RegisterVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool SimConsumption::Iterate()
{
  AppCastingMOOSApp::Iterate();
  double delay = MOOSTime() - m_previous_iteration_date;

  if(m_no_acceleration_no_deceleration)
  {
    if(m_thrust_force > 1.0)
      m_power_prop = 90.;

    else
      m_power_prop = 0.;
  }

  else
  {
    m_power_prop = m_thrust_force * m_surge_speed;
    m_power_prop *= 1 / m_efficiency_motor;
    m_power_prop *= 1 / m_efficiency_gear;
    m_power_prop *= 1 / m_efficiency_shaft;
    m_power_prop *= 1 / m_efficiency_prop;
  }


  //PLACEHOLDER FOR POWER CONSUMPTION WHEN TURNING!
  if(fabs(m_heading_thrust) > 20.0) m_power_prop = 96.;
  //-----------------------------------------------

  Notify("POWER_PROP", m_power_prop);
  Notify("POWER_NOMINAL", m_power_nominal);

  // Integration: power -> conso

  m_conso_prop += m_power_prop * delay;
  m_conso_nominal += m_power_nominal * delay;

  Notify("CONSO_PROP", m_conso_prop);
  Notify("CONSO_NOMINAL", m_conso_nominal);
  Notify("CONSO_ALL", GetGlobalConsumption());
  Notify("ESTIMATION_DURATION_DAYS", (m_energy_pack * (m_db_uptime / (3600*24)) / (GetGlobalConsumption() / 3600.)));

  if (((m_energy_pack - (GetGlobalConsumption() / 3600.)) <= 0) && (!m_energy_pack_expended)) {
    Notify("ENERGY PACK EXPENDED", true);
    Notify("ESTIMATION_DURATION_DAYS_FINAL", (m_energy_pack * (m_db_uptime / (3600*24)) / (GetGlobalConsumption() / 3600.)));
    Notify("MISSION_DURATION_DAYS_FINAL", m_db_uptime/(3600*24) + (m_energy_pack - (GetGlobalConsumption() / 3600.))/24);
    m_final_duration = (m_db_uptime/(3600*24) + (m_energy_pack - (GetGlobalConsumption() / 3600.))/24);
    m_energy_pack_expended = true;
  }

  m_previous_iteration_date = MOOSTime();

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool SimConsumption::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
  {
    sParams.reverse(); // EFFICIENCY_ have to be read before POWER_SPEED
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++)
    {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);

      if(param == "EFFICIENCY_PROP")
        m_efficiency_prop = atof(value.c_str());

      else if(param == "EFFICIENCY_SHAFT")
        m_efficiency_shaft = atof(value.c_str());

      else if(param == "EFFICIENCY_GEAR")
        m_efficiency_gear = atof(value.c_str());

      else if(param == "EFFICIENCY_MOTOR")
        m_efficiency_motor = atof(value.c_str());

      else if(param == "POWER_NOMINAL")
        m_power_nominal = atof(value.c_str());

      else if(param == "CONSUMPTION_PER_PING")
        m_consumption_per_ping = atof(value.c_str());

      else if(param == "POWER_SPEED")
        HandlePowerSpeed(value); // to use with uSimMarine

      else if(param == "ENERGY_PACK")
        m_energy_pack = atof(value.c_str());

      else if(param == "DECAYING_SPEED_RATIO")
      {
        setBooleanOnString(m_no_acceleration_no_deceleration, value);
        m_no_acceleration_no_deceleration = !m_no_acceleration_no_deceleration;
      }
    }
  }

  RegisterVariables();
  m_previous_iteration_date = MOOSTime();

  return true;
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void SimConsumption::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("PING", 0.0);
  Register("NAV_THRUST_FORCE", 0.0);
  Register("NAV_SURGE_SPEED", 0.0);
  Register("NAV_SPEED", 0.0); // to use with uSimMarine
  Register("DB_UPTIME", 0.0);
  //PLACEHOLDER FOR PITCH, SURGE AND HEADING THRUSTS, FOR POWER CONSUMPTION WHEN TURNING!
  Register("NAV_HEADING_THRUST", 0.0);
  //---------------------------------------
}

//---------------------------------------------------------
// Procedure: HandleNewPing

bool SimConsumption::HandleNewPing(CMOOSMsg msg)
{
  string ping_value = msg.GetString();
  string community_name = msg.GetCommunity();

  if(community_name == m_host_community)
  {
    m_pings_number ++;
    m_conso_comm = m_pings_number * m_consumption_per_ping;
    Notify("CONSO_COMM", m_conso_comm);
  }

  return true;
}


//---------------------------------------------------------
// Procedure: HandlePowerSpeed - to use with uSimMarine

void SimConsumption::HandlePowerSpeed(string value)
{
  double power = 0.;
  double speed = 0.;
  MOOSValFromString(power, value, "power");
  MOOSValFromString(speed, value, "speed");

  if(speed > 0.)
  {
    double meca_power = power * m_efficiency_prop * m_efficiency_shaft * m_efficiency_gear * m_efficiency_motor;
    double drag = meca_power / speed;
    m_drag_coef = drag / (speed * fabs(speed));
  }

  else
    cout << "Unhandled POWER_SPEED parameter : speed must be > 0" << endl;
}

//---------------------------------------------------------
// Procedure: GetGlobalConsumption

double SimConsumption::GetGlobalConsumption()
{
  return m_conso_prop + m_conso_comm + m_conso_nominal;
}

//---------------------------------------------------------
// Procedure: BuildReport

bool SimConsumption::buildReport()
{
  m_msgs << endl;

  m_msgs << "Power:" << endl;
  m_msgs << "\t- nominal: \t\t" << m_power_nominal << " W" << endl;
  m_msgs << "\t- propulsion: \t\t" << m_power_prop << " W" << endl;
  m_msgs << "\t- communication: \t" << m_pings_number << " pings" << endl << endl;

  m_msgs << "Consumption:" << endl;
  m_msgs << "\t- nominal: \t\t" << (m_conso_nominal / 3600.) << " WH" << endl;
  m_msgs << "\t- propulsion: \t\t" << (m_conso_prop / 3600.) << " WH" << endl;
  m_msgs << "\t- communication: \t" << (m_conso_comm / 3600.) << " WH" << endl;
  m_msgs << "\t- total: \t\t" << (GetGlobalConsumption() / 3600.) << " WH" << endl;
  m_msgs << "\t- remainder: \t\t" << m_energy_pack - (GetGlobalConsumption() / 3600.) << " WH" << endl << endl;

  m_msgs << "Duration:" << endl;
  m_msgs << "\t- Mission duration estimation: \t\t" << (m_energy_pack * (m_db_uptime / (3600*24)) / (GetGlobalConsumption() / 3600.)) << " days" << endl;
  m_msgs << "\t- Mission elapsed: \t\t\t" << m_db_uptime/(3600*24) << " days" << endl;
  m_msgs << "\t- Mission remaining (estimated): \t" << (m_energy_pack - (GetGlobalConsumption() / 3600.))/24 << " days" << endl;
  m_msgs << "\t- Mission elapsed + remaining: \t\t" << m_db_uptime/(3600*24) + (m_energy_pack - (GetGlobalConsumption() / 3600.))/24 << " days" << endl << endl;

  if (m_energy_pack_expended) {
    m_msgs << "Energy Pack Expended!:" << endl;
    m_msgs << "\t- Above duration calculations no longer accurate!" << endl;
    m_msgs << "\t- Mission duration TOTAL: \t\t" << m_final_duration << " days" << endl << endl;
  }

  m_msgs << "Input:" << endl;
  m_msgs << "\t- thrust_force: \t" << m_thrust_force << " N" << endl;
  m_msgs << "\t- surge_speed: \t\t" << m_surge_speed << " m/s" << endl << endl;

  m_msgs << "Set Parameters:" << endl;
  m_msgs << "\t- energy_pack: \t\t" << m_energy_pack << " WH" << endl;
  m_msgs << "\t- efficiency_prop: \t" << m_efficiency_prop << endl;
  m_msgs << "\t- efficiency_shaft: \t" << m_efficiency_shaft << endl;
  m_msgs << "\t- efficiency_gear: \t" << m_efficiency_gear << endl;
  m_msgs << "\t- efficiency_motor: \t" << m_efficiency_motor << endl;
  m_msgs << "\t- power_nominal: \t" << m_power_nominal << " W" << endl;
  m_msgs << "\t- consumption_per_ping: " << (m_consumption_per_ping / 3600.) << " WH" << endl;

  return true;
}
