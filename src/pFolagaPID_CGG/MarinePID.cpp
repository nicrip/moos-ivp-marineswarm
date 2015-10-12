/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: MarinePID.cpp                                        */
/*    DATE: Apr 10 2006                                          */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/
/*                                                               */
/* Modified by :  Nathan Vandervaeren                            */
/* Date :         April 2014                                     */
/* Application :  pFolagaPID_CGG                                 */
/*                                                               */
/*****************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#endif
#include <iterator>
#include <iostream>
#include <cmath>
#include <cstring>
#include "MarinePID.h"
#include "MBUtils.h"

using namespace std;

#define VERBOSE

//--------------------------------------------------------------------
// Procedure: Constructor

MarinePID::MarinePID()
{
  m_has_control    = false;
  m_allow_overide  = true;
  m_allstop_posted = false;
  
  m_depth_control  = true;
  m_spdPID_active  = false;
  m_verbose        = "terse";

  m_desired_heading = 0.0;
  m_desired_speed   = 0.0;
  m_desired_depth   = 0.0;

  m_current_heading = 0.0;
  m_current_speed   = 0.0;
  m_current_depth   = 0.0;
  m_current_pitch   = 0.0;
  m_current_surge_thrust = 0.0;

  m_max_surge_thrust      = 100.0;
  m_max_heading_thrust    = 100.0;
  m_max_pitch_thrust      = 100.0;
  m_max_pitch_thrust      = 18.0;
  m_max_speed             = 1.0;
  m_heading_limit         = 90.0;
  
  m_iteration    = 0;
  m_start_time   = 0.0;

  m_paused       = false;

  m_tardy_helm_thresh = 2.0;
  m_tardy_nav_thresh  = 2.0;

  m_time_of_last_helm_msg = 0.0;
  m_time_of_last_nav_msg  = 0.0;
}


//--------------------------------------------------------------------
// Procedure: OnNewMail()

bool MarinePID::OnNewMail(MOOSMSG_LIST &NewMail)
{
  double curr_time = MOOSTime();

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    double dfT;

    msg.IsSkewed(curr_time, &dfT);

    string key = toupper(stripBlankEnds(msg.m_sKey));
    
    #define ACCEPTABLE_SKEW_FIX_ME 360.0
    if(fabs(dfT)<ACCEPTABLE_SKEW_FIX_ME) {
      if((key == "MOOS_MANUAL_OVERIDE") ||
	 (key == "MOOS_MANUAL_OVERRIDE")) {
	if(MOOSStrCmp(msg.m_sVal, "FALSE")) {
	  m_has_control = true;
	  MOOSTrace("\n");
	  MOOSDebugWrite("pMarinePID Control Is On");
	}
	else if(MOOSStrCmp(msg.m_sVal, "TRUE")) {
	  if(m_allow_overide) {
	    m_has_control = false;
	    MOOSTrace("\n");
	    MOOSDebugWrite("pMarinePID Control Is Off");
	  }
	}
      }
      else if(key == "PID_VERBOSE") {
	if(msg.m_sVal == "verbose")
	  m_verbose = "verbose";
	else if(msg.m_sVal == "quiet")
	  m_verbose = "quiet";
	else
	  m_verbose = "terse";
      }
      else if(key == "NAV_HEADING")
        m_current_heading = msg.m_dfVal;
      else if(key == "NAV_SPEED")
        m_current_speed = msg.m_dfVal;
      else if(key == "NAV_DEPTH")
        m_current_depth = msg.m_dfVal;
      else if(key == "NAV_PITCH")
        m_current_pitch = msg.m_dfVal;
      
      if(!strncmp(key.c_str(), "NAV_", 4))
        m_time_of_last_nav_msg = curr_time;

      else if(key == "DESIRED_SURGE_THRUST")
        m_current_surge_thrust = msg.m_dfVal;
      else if(key == "DESIRED_HEADING") {
        m_desired_heading = msg.m_dfVal;
        m_time_of_last_helm_msg = curr_time;
        }
      else if(key == "DESIRED_SPEED") {
        m_desired_speed = msg.m_dfVal;
        m_time_of_last_helm_msg = curr_time;
      }
      else if(key == "DESIRED_DEPTH") {
        m_desired_depth = msg.m_dfVal;
        m_time_of_last_helm_msg = curr_time;
      }
    }
  }
  return(true);
}

//--------------------------------------------------------------------
// Procedure: Iterate()

bool MarinePID::Iterate()
{
  m_iteration++;
  postCharStatus();

  if(!m_has_control) {
    postAllStop();
    return(false);
  }

  double current_time = MOOSTime();

  if(m_verbose == "verbose") {
    double hz = m_iteration / (MOOSTime() - m_start_time);
    cout << endl << endl << endl;
    cout << "PID REPORT: (" << m_iteration << ")";
    cout << "(" << hz << "/sec)" << endl;
  }

  if((current_time - m_time_of_last_helm_msg) > m_tardy_helm_thresh) {
    if(!m_paused)
      MOOSDebugWrite("Paused Due To Tardy HELM Input: ALL OUTPUTS=0");
    cout << "Paused Due To Tardy HELM Input: ALL OUTPUTS=0" << endl;
    m_paused = true;
    Notify("DESIRED_SURGE_THRUST", 0.0);
    Notify("DESIRED_HEADING_THRUST", 0.0);
    Notify("DESIRED_PITCH_THRUST", 0.0);
    return(true);
  }
  
  if((current_time - m_time_of_last_nav_msg) > m_tardy_nav_thresh) {
    if(!m_paused)
      MOOSDebugWrite("Paused Due To Tardy NAV Input: ALL OUTPUTS=0");
    cout << "Paused Due To Tardy NAV Input: ALL OUTPUTS=0" << endl;
    m_paused = true;
    Notify("DESIRED_SURGE_THRUST", 0.0);
    Notify("DESIRED_HEADING_THRUST", 0.0);
    Notify("DESIRED_PITCH_THRUST", 0.0);
    return(true);
  }

  double surge_thrust   = 0;
  double heading_thrust = 0;
  double pitch_thrust   = 0;

  m_pengine.updateTime(current_time);

  heading_thrust = m_pengine.getDesiredHeadingThrust(m_desired_heading, m_current_heading, 
				    m_max_heading_thrust);

  surge_thrust = m_pengine.getDesiredSurgeThrust(m_desired_speed, m_current_speed, 
				    m_current_surge_thrust, m_max_surge_thrust, m_max_speed, m_current_heading, m_desired_heading, m_heading_limit, m_drag_map, m_spdPID_active);
            
  if(m_depth_control)
    pitch_thrust = m_pengine.getDesiredPitchThrust(m_desired_depth, m_current_depth,
					  m_current_pitch, m_max_pitch, 
					  m_max_pitch_thrust);
  
  if((m_desired_speed <= 0.001) && (m_desired_speed >= -0.001))
    surge_thrust = 0;

  vector<string> pid_report;
  if(m_verbose == "verbose") {
    pid_report = m_pengine.getPIDReport();
    for(unsigned int i=0; i<pid_report.size(); i++)
      cout << pid_report[i] << endl;
  }
  m_pengine.clearReport();

  m_paused = false;

  Notify("DESIRED_HEADING_THRUST", heading_thrust);
  Notify("DESIRED_SURGE_THRUST", surge_thrust);
  if(m_depth_control)
    Notify("DESIRED_PITCH_THRUST", pitch_thrust);

  m_allstop_posted = false;
  
  return(true);
}
  

//------------------------------------------------------------
// Procedure: postCharStatus()

void MarinePID::postCharStatus()
{
  if(m_has_control) {
    if(m_verbose == "terse")
      MOOSTrace("$");
  }
  else
    MOOSTrace("*");
}

//------------------------------------------------------------
// Procedure: postAllStop()

void MarinePID::postAllStop()
{
  if(m_allstop_posted)
    return;

  Notify("DESIRED_HEADING_THRUST", 0.0);
  Notify("DESIRED_SURGE_THRUST", 0.0);
  if(m_depth_control)
    Notify("DESIRED_PITCH_THRUST", 0.0);

  m_allstop_posted = true;
}

//------------------------------------------------------------
// Procedure: OnConnectToServer()

bool MarinePID::OnConnectToServer()
{
  registerVariables();
  return(true);
}

//------------------------------------------------------------
// Procedure: registerVariables

void MarinePID::registerVariables()
{
  m_Comms.Register("NAV_HEADING", 0);
  m_Comms.Register("NAV_SPEED", 0);
  m_Comms.Register("NAV_DEPTH", 0);
  m_Comms.Register("NAV_PITCH", 0);
  m_Comms.Register("DESIRED_HEADING", 0);
  m_Comms.Register("DESIRED_SPEED", 0);
  m_Comms.Register("DESIRED_SURGE_THRUST", 0);
  m_Comms.Register("DESIRED_DEPTH", 0);
  m_Comms.Register("PID_VERBOSE", 0);
  m_Comms.Register("MOOS_MANUAL_OVERIDE", 0);
  m_Comms.Register("MOOS_MANUAL_OVERRIDE", 0);
}

//--------------------------------------------------------
// Procedure: onStartUp()

bool MarinePID::OnStartUp()
{
  cout << "pMarinePID starting...." << endl;
  
  m_start_time = MOOSTime();
  
  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
  
  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string sLine = *p;
    string param = toupper(biteStringX(sLine, '='));
    string value = sLine;
    double dval  = atof(value.c_str());
    
    if(param == "DRAG_MAP")
      m_drag_map.setPoints(value);
      
    else if(param == "HEADING_LIMIT")
      m_heading_limit = dval;
      
     else if(param == "SPD_PID_ACTIVE") {
      if(sLine == "true")
        m_spdPID_active = true;
      else
        m_spdPID_active = false;
      }
      
    else if(param == "TARDY_HELM_THRESHOLD") 
      m_tardy_helm_thresh = vclip_min(dval, 0);
    else if(param == "TARDY_NAV_THRESHOLD")
      m_tardy_nav_thresh = vclip_min(dval, 0);
    else if(param == "ACTIVE_START") 
      setBooleanOnString(m_has_control, value);
    else if(param == "VERBOSE") {
      if((sLine == "true") || (sLine == "verbose"))
	m_verbose = "verbose";
      if(sLine == "terse") 
	m_verbose = "terse";
      if(sLine == "quiet")
	m_verbose = "quiet";
    }
  }

  bool ok_yaw = handleYawSettings();
  bool ok_spd = handleSpeedSettings();
  bool ok_dep = handleDepthSettings();

  if(!ok_yaw || !ok_spd || !ok_dep) {
    cout << "Improper PID Settings" << endl;
    return(false);
  }

  if(m_depth_control)
    cout << "Depth Control is ON" << endl;
  else
    cout << "Depth Control is OFF" << endl;

  registerVariables();
  
  return(true);
}

//--------------------------------------------------------------------
// Procedure: handleYawSettings

bool MarinePID::handleYawSettings()
{
  int ok = true;

  double yaw_pid_Kp, yaw_pid_Kd, yaw_pid_Ki, yaw_pid_ilim;
  if(!m_MissionReader.GetConfigurationParam("YAW_PID_KP", yaw_pid_Kp)) {
    MOOSDebugWrite("YAW_PID_KP not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("YAW_PID_KD", yaw_pid_Kd)) {
    MOOSDebugWrite("YAW_PID_KD not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("YAW_PID_KI", yaw_pid_Ki)) {
    MOOSDebugWrite("YAW_PID_KI not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("YAW_PID_INTEGRAL_LIMIT", yaw_pid_ilim)) {
    MOOSDebugWrite("YAW_PID_INTEGRAL_LIMIT not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("MAX_HEADING_THRUST",m_max_heading_thrust)) {
    MOOSDebugWrite("MAX_HEADING_THRUST not found in Mission File");
    ok = false;
  }
  
  ScalarPID crsPID;
  crsPID.SetGains(yaw_pid_Kp, yaw_pid_Kd, yaw_pid_Ki);
  crsPID.SetLimits(yaw_pid_ilim, 100);
  m_pengine.setPID(0, crsPID);

  MOOSDebugWrite(MOOSFormat("** NEW CONTROLLER GAINS ARE **"));
  MOOSDebugWrite(MOOSFormat("YAW_PID_KP             = %.3f",yaw_pid_Kp));
  MOOSDebugWrite(MOOSFormat("YAW_PID_KD             = %.3f",yaw_pid_Kd));
  MOOSDebugWrite(MOOSFormat("YAW_PID_KI             = %.3f",yaw_pid_Ki));
  MOOSDebugWrite(MOOSFormat("YAW_PID_INTEGRAL_LIMIT = %.3f",yaw_pid_ilim));
  MOOSDebugWrite(MOOSFormat("MAX_HEADING_THRUST     = %.3f",m_max_heading_thrust));
  
  return(ok);
}
  

//--------------------------------------------------------------------
// Procedure: handleSpeedSettings

bool MarinePID::handleSpeedSettings()
{
  int ok = true;

  double spd_pid_Kp, spd_pid_Kd, spd_pid_Ki, spd_pid_ilim;
  if(!m_MissionReader.GetConfigurationParam("SPEED_PID_KP", spd_pid_Kp)) {
    MOOSDebugWrite("SPEED_PID_KP not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("SPEED_PID_KD", spd_pid_Kd)) {
    MOOSDebugWrite("SPEED_PID_KD not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("SPEED_PID_KI", spd_pid_Ki)) {
    MOOSDebugWrite("SPEED_PID_KI not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("SPEED_PID_INTEGRAL_LIMIT", spd_pid_ilim)) {
    MOOSDebugWrite("SPEED_PID_INTEGRAL_LIMIT not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("MAX_SURGE_THRUST",m_max_surge_thrust))
  {
    MOOSDebugWrite("MAX_SURGE_THRUST not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("MAX_SPEED",m_max_speed))
  {
    MOOSDebugWrite("MAX_SPEED not found in Mission File");
    ok = false;
  }

  ScalarPID spdPID;
  spdPID.SetGains(spd_pid_Kp, spd_pid_Kd, spd_pid_Ki);
  spdPID.SetLimits(spd_pid_ilim, 100);
  m_pengine.setPID(1, spdPID);

  MOOSDebugWrite(MOOSFormat("SPEED_PID_KP           = %.3f",spd_pid_Kp));
  MOOSDebugWrite(MOOSFormat("SPEED_PID_KD           = %.3f",spd_pid_Kd));
  MOOSDebugWrite(MOOSFormat("SPEED_PID_KI           = %.3f",spd_pid_Ki));
  MOOSDebugWrite(MOOSFormat("SPEED_PID_KI_LIMIT     = %.3f",spd_pid_ilim));
  MOOSDebugWrite(MOOSFormat("MAX_SURGE_THRUST       = %.3f",m_max_surge_thrust));
  MOOSDebugWrite(MOOSFormat("MAX_SPEED              = %.3f",m_max_speed));

  return(ok);
}

//--------------------------------------------------------------------
// Procedure: handleDepthSettings()

bool MarinePID::handleDepthSettings()
{
  int ok = true;


#if 1
  string depth_control_str = "false";
  m_MissionReader.GetConfigurationParam("DEPTH_CONTROL", depth_control_str);
  depth_control_str = tolower(depth_control_str);
  m_depth_control = ((depth_control_str == "true") ||
		   (depth_control_str == "1") ||
		   (depth_control_str == "yes"));
#endif

#if 0
  m_MissionReader.GetConfigurationParam("DEPTH_CONTROL", m_depth_control);
#endif
#if 0
  double dc;
  m_MissionReader.GetConfigurationParam("DEPTH_CONTROL", dc);
  m_depth_control = (int)dc;
#endif
  
  if(!m_depth_control)
    return(true);
  

  double z_top_pid_Kp, z_top_pid_Kd, z_top_pid_Ki, z_top_pid_ilim;
  if(!m_MissionReader.GetConfigurationParam("Z_TO_PITCH_PID_KP", z_top_pid_Kp)) {
    MOOSDebugWrite("Z_TO_PITCH_PID_KP not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("Z_TO_PITCH_PID_KD", z_top_pid_Kd)) {
    MOOSDebugWrite("Z_TO_PITCH_PID_KD not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("Z_TO_PITCH_PID_KI", z_top_pid_Ki)) {
    MOOSDebugWrite("Z_TO_PITCH_PID_KI not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("Z_TO_PITCH_PID_INTEGRAL_LIMIT", z_top_pid_ilim)) {
    MOOSDebugWrite("Z_TO_PITCH_PID_INTEGRAL_LIMIT not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("MAX_PITCH",m_max_pitch))
  {
    MOOSDebugWrite("MAX_PITCH not found in Mission File");
    ok = false;
  }

  ScalarPID ztopPID;
  ztopPID.SetGains(z_top_pid_Kp, z_top_pid_Kd, z_top_pid_Ki);
  ztopPID.SetLimits(z_top_pid_ilim, 100);
  m_pengine.setPID(2, ztopPID);

  MOOSDebugWrite(MOOSFormat("Z_TO_PITCH_PID_KP              = %.3f",z_top_pid_Kp));
  MOOSDebugWrite(MOOSFormat("Z_TO_PITCH_PID_KD              = %.3f",z_top_pid_Kd));
  MOOSDebugWrite(MOOSFormat("Z_TO_PITCH_PID_KI              = %.3f",z_top_pid_Ki));
  MOOSDebugWrite(MOOSFormat("Z_TO_PITCH_PID_INTEGRAL_LIMIT  = %.3f",z_top_pid_ilim));
  MOOSDebugWrite(MOOSFormat("MAX_PITCH                      = %.3f",m_max_pitch));
  
  double pitch_pid_Kp, pitch_pid_Kd, pitch_pid_Ki, pitch_pid_ilim;
  if(!m_MissionReader.GetConfigurationParam("PITCH_PID_KP", pitch_pid_Kp)) {
    MOOSDebugWrite("PITCH_PID_KP not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("PITCH_PID_KD", pitch_pid_Kd)) {
    MOOSDebugWrite("PITCH_PID_KD not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("PITCH_PID_KI", pitch_pid_Ki)) {
    MOOSDebugWrite("PITCH_PID_KI not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("PITCH_PID_INTEGRAL_LIMIT", pitch_pid_ilim)) {
    MOOSDebugWrite("PITCH_PID_INTEGRAL_LIMIT not found in Mission File");
    ok = false;
  }
  if(!m_MissionReader.GetConfigurationParam("MAX_PITCH_THRUST",m_max_pitch_thrust))
  {
    MOOSDebugWrite("MAX_PITCH_THRUST not found in Mission File");
    ok = false;
  }

  ScalarPID pitchPID;
  pitchPID.SetGains(pitch_pid_Kp, pitch_pid_Kd, pitch_pid_Ki);
  pitchPID.SetLimits(pitch_pid_ilim, 100);
  m_pengine.setPID(3, pitchPID);

  MOOSDebugWrite(MOOSFormat("PITCH_PID_KP           = %.3f",pitch_pid_Kp));
  MOOSDebugWrite(MOOSFormat("PITCH_PID_KD           = %.3f",pitch_pid_Kd));
  MOOSDebugWrite(MOOSFormat("PITCH_PID_KI           = %.3f",pitch_pid_Ki));
  MOOSDebugWrite(MOOSFormat("PITCH_PID_KI_LIMIT     = %.3f",pitch_pid_ilim));
  MOOSDebugWrite(MOOSFormat("MAX_PITCH_THRUST       = %.3f",m_max_pitch_thrust));

  return(ok);
}


