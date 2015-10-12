/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USM_MOOSApp.cpp                                      */
/*    DATE: Oct 25th 2004                                        */
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
/* Application :  uSimFolaga_CGG                                 */
/*                                                               */
/*****************************************************************/

#include <iostream>
#include <math.h>
#include "USM_MOOSApp.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "AngleUtils.h"

#define USE_UTM

using namespace std;

//------------------------------------------------------------------------
// Constructor

USM_MOOSApp::USM_MOOSApp() 
{
  m_sim_prefix  = "USM";
  m_reset_count = 0;
  m_geo_ok      = false;

  m_last_report = 0;
  m_report_interval = 5;
}


//------------------------------------------------------------------------
// Procedure: OnNewMail

bool USM_MOOSApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
  
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key = msg.GetKey();
    double dval = msg.GetDouble();
    string sval = msg.GetString();

    // Control inputs
    
    if(key == "DESIRED_SURGE_THRUST")
      m_model.setSurgeThrust(dval);
    else if(key == "DESIRED_HEADING_THRUST")
      m_model.setHeadingThrust(dval);
    else if(key == "DESIRED_PITCH_THRUST")
      m_model.setPitchThrust(dval);
    else if(key == "USM_SIM_PAUSED")
      m_model.setPaused(toupper(sval) == "TRUE");

    // Current inputs
    
    else if(key == "DRIFT_X")
      m_model.setDriftX(dval);
    else if(key == "DRIFT_Y")
      m_model.setDriftY(dval);
    else if(key == "DRIFT_Z")
      m_model.setDriftZ(dval);
      
    // Water depth

    else if(key == "WATER_DEPTH")
      m_model.setWaterDepth(dval);

    else if(key == "USM_RESET") {
      m_reset_count++;
      Notify("USM_RESET_COUNT", m_reset_count);
      m_model.initPosition(sval);
    }
    else 
      reportRunWarning("Unhandled mail: " + key);
  }
  
  return(true);
}
  
  
//------------------------------------------------------------------------
// Procedure: OnStartUp
//      Note: 

bool USM_MOOSApp::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  
  m_model.resetTime(m_curr_time);

  STRING_LIST sParams;
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams)) 
    reportConfigWarning("No config block found for " + GetAppName());
    
  STRING_LIST::iterator p;
  for(p = sParams.begin();p!=sParams.end();p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    double dval  = atof(value.c_str());

    bool handled = false;
    
    // Initial position
    if(param == "START_POS")
      handled = m_model.initPosition(value);
    else if((param == "START_X") && isNumber(value))
      handled = m_model.setParam(param, dval);
    else if((param == "START_Y") && isNumber(value))
      handled = m_model.setParam(param, dval);
    else if((param == "START_HEADING") && isNumber(value))
      handled = m_model.setParam(param, dval);
    else if((param == "START_SPEED") && isNumber(value))
      handled = m_model.setParam(param, dval);
    else if((param == "START_DEPTH") && isNumber(value))
      handled = m_model.setParam(param, dval);
      
      
    // Initial current
    else if((param == "START_DRIFT_X") && isNumber(value))
      handled = m_model.setParam("drift_x", dval);
    else if((param == "START_DRIFT_Y") && isNumber(value))
      handled = m_model.setParam("drift_y", dval);
    else if((param == "START_DRIFT_Z") && isNumber(value))
      handled = m_model.setParam("drift_z", dval);
      
    // Characteristics
    else if((param == "PREFIX") && !strContainsWhite(value)) {      
      m_sim_prefix = value;
      handled = true;
    }
    else if((param == "MASS") && isNumber(value))
      handled = m_model.setParam("mass", dval);
    else if(param == "DRAG_MAP")
      handled = m_model.handleDragMap(value);
    else if((param == "MAX_SPEED") && isNumber(value))
      handled = m_model.setParam("max_speed", dval);  
    else if((param == "MAX_ROTATE_SPEED") && isNumber(value))
      handled = m_model.setParam("max_rotate_speed", dval);
      
    // For heading noise  
    else if((param == "HEADING_SIGMA") && isNumber(value))
      handled = m_model.setParam("heading_sigma", dval);
    else if((param == "HEADING_MU") && isNumber(value))
      handled = m_model.setParam("heading_mu", dval);
    else if((param == "HEADING_SATURATION") && isNumber(value))
      handled = m_model.setParam("heading_saturation", dval);
    
    else if((param == "SIM_PAUSE") && isBoolean(value)) {
      m_model.setPaused(tolower(value) == "true");
      handled = true;
    }
    
    // Default water depth
    else if((param == "DEFAULT_WATER_DEPTH") && isNumber(value))
      handled = m_model.setParam("water_depth", dval);
        
    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  // look for latitude, longitude global variables
  double latOrigin, longOrigin;
  if(!m_MissionReader.GetValue("LatOrigin", latOrigin)) {
    MOOSTrace("uSimFolaga: LatOrigin not set in *.moos file.\n");
    m_geo_ok = false;
  } 
  else if(!m_MissionReader.GetValue("LongOrigin", longOrigin)) {
    MOOSTrace("uSimFolaga: LongOrigin not set in *.moos file\n");
    m_geo_ok = false;      
  }
  else {
    m_geo_ok = true;
    // initialize m_geodesy
    if(!m_geodesy.Initialise(latOrigin, longOrigin)) {
      MOOSTrace("uSimFolaga: Geodesy init failed.\n");
      m_geo_ok = false;
    }
  }
 
  cacheStartingInfo();
 
  registerVariables();
  MOOSTrace("uSimFolaga started \n");
  return(true);
}


//------------------------------------------------------------------------
// Procedure: OnConnectToServer
//      Note: 

bool USM_MOOSApp::OnConnectToServer()
{
  registerVariables();
  MOOSTrace("SimFolaga connected\n");
  
  return(true);
}


//------------------------------------------------------------------------
// Procedure: cacheStartingInfo()

void USM_MOOSApp::cacheStartingInfo()
{
  NodeRecord record = m_model.getNodeRecord();
  double nav_x = record.getX();
  double nav_y = record.getY();
  double water_depth = m_model.getWaterDepth();
  double nav_depth = record.getDepth();
  double nav_alt = water_depth - nav_depth;

  double nav_lat, nav_lon;
 if(m_geo_ok) {
#ifdef USE_UTM
    m_geodesy.UTM2LatLong(nav_x, nav_y, nav_lat, nav_lon);
#else
    m_geodesy.LocalGrid2LatLong(nav_x, nav_y, nav_lat, nav_lon);
#endif
 }

  m_start_nav_x     = doubleToStringX(nav_x,2);
  m_start_nav_y     = doubleToStringX(nav_y,2);
  m_start_nav_lat   = doubleToStringX(nav_lat,8);
  m_start_nav_lon   = doubleToStringX(nav_lon,8);
  m_start_nav_spd   = doubleToStringX(record.getSpeed(),2);
  m_start_nav_hdg   = doubleToStringX(record.getHeading(),1);
  m_start_nav_ptc   = doubleToStringX(record.getPitch(),2);
  m_start_nav_dep   = doubleToStringX(record.getDepth(),2);
  m_start_nav_alt   = doubleToStringX(nav_alt,2);
  m_start_drift_x   = doubleToStringX(m_model.getDriftX(),4);
  m_start_drift_y   = doubleToStringX(m_model.getDriftY(),4);
  m_start_drift_z   = doubleToStringX(m_model.getDriftZ(),4);
  m_start_drift_mag = doubleToStringX(m_model.getDriftMag(),4);
  m_start_drift_ang = doubleToStringX(m_model.getDriftAng(),4);
}


//------------------------------------------------------------------------
// Procedure: registerVariables

void USM_MOOSApp::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  m_Comms.Register("DESIRED_SURGE_THRUST", 0);
  m_Comms.Register("DESIRED_HEADING_THRUST", 0);
  m_Comms.Register("DESIRED_PITCH_THRUST", 0);

  m_Comms.Register("WATER_DEPTH", 0);

  m_Comms.Register("DRIFT_X",0);
  m_Comms.Register("DRIFT_Y",0);
  m_Comms.Register("DRIFT_Z",0);

  m_Comms.Register("USM_SIM_PAUSED", 0); 
  m_Comms.Register("USM_RESET", 0);
}


//------------------------------------------------------------------------
// Procedure: Iterate
//      Note: This is where it all happens.

bool USM_MOOSApp::Iterate()
{
  AppCastingMOOSApp::Iterate();
  m_model.propagate(m_curr_time);
  
  NodeRecord record = m_model.getNodeRecord();
  
  postNodeRecordUpdate(m_sim_prefix, record);

  //PLACEHOLDER FOR PITCH, SURGE AND HEADING THRUSTS, FOR POWER CONSUMPTION WHEN TURNING!
  Notify(m_sim_prefix+"_HEADING_THRUST", m_model.getHeadingThrust(), m_curr_time);
  Notify(m_sim_prefix+"_SURGE_THRUST", m_model.getSurgeThrust(), m_curr_time);
  Notify(m_sim_prefix+"_PITCH_THRUST",m_model.getPitchThrust(), m_curr_time);
  //----------------------------------------//

  Notify("USM_DRIFT_SUMMARY", m_model.getDriftSummary());

  AppCastingMOOSApp::PostReport();
  return(true);
}


//------------------------------------------------------------------------
// Procedure: postNodeRecordUpdate

void USM_MOOSApp::postNodeRecordUpdate(string prefix, 
				       const NodeRecord &record)
{
  double nav_x = record.getX();
  double nav_y = record.getY();

  Notify(prefix+"_X", nav_x, m_curr_time);
  Notify(prefix+"_Y", nav_y, m_curr_time);

  Notify(prefix+"_X_MOD30", fmod(nav_x, 30), m_curr_time);
  Notify(prefix+"_Y_MOD30", fmod(nav_y, 30), m_curr_time);

  
  if(m_geo_ok) {
    double lat, lon;
#ifdef USE_UTM
    m_geodesy.UTM2LatLong(nav_x, nav_y, lat, lon);
#else
    m_geodesy.LocalGrid2LatLong(nav_x, nav_y, lat, lon);
#endif
    Notify(prefix+"_LAT", lat, m_curr_time);
    Notify(prefix+"_LONG", lon, m_curr_time);
  }

  double new_speed = record.getSpeed();
  new_speed = snapToStep(new_speed, 0.01);

  Notify(prefix+"_HEADING", record.getHeading(), m_curr_time);
  Notify(prefix+"_SPEED", new_speed, m_curr_time);
  Notify(prefix+"_DEPTH", record.getDepth(), m_curr_time);

  Notify(prefix+"_Z", -record.getDepth(), m_curr_time);
  Notify(prefix+"_PITCH", record.getPitch(), m_curr_time);

  double hog = angle360(record.getHeadingOG());
  double sog = record.getSpeedOG();

  Notify(prefix+"_HEADING_OVER_GROUND", hog, m_curr_time);
  Notify(prefix+"_SPEED_OVER_GROUND", sog, m_curr_time);
  
  if(record.isSetAltitude()) 
    Notify(prefix+"_ALTITUDE", record.getAltitude(), m_curr_time);
  
}


//------------------------------------------------------------------------
// Procedure: buildReport
//      Note: A virtual function of the AppCastingMOOSApp superclass, 
//            conditionally invoked if either a terminal or appcast 
//            report is needed.
//
// Datum: 43.825300,  -71.087589, (MIT Sailing Pavilion)
//
//   Starting Pose                Current Pose
//   -------- ----------         -------- -----------
//   Heading: 180                Heading: 134.8
//     Speed: 0                    Speed: 1.2
//     Depth: 0                    Depth: 37.2  
//  Altitude: 58                Altitude: 20.8
//     (X,Y): 0,0                  (X,Y): -4.93,-96.05
//       Lat: 43.8253                Lat: 43.82443465       
//       Lon: -70.3304               Lon: -70.33044214      

//  External Drift  X   Y   |  Mag  Ang  |  Rotate  |  Source(s)  
//  --------------  --  --  |  ---  ---  |  ------  |  -----------
//         Present  0   0   |  0    0    |  0       |  n/a        
// 
// Velocity Information :
// ----------------------
//  DESIRED_SURGE_THRUST = 55.3
//
//  DESIRED_HEADING_THRUST = 20.5
//
//  DESIRED_PITCH_THRUST = 0
//
//  Positive Drag Map: 0:1, 20:2.4, 50:4.2, 80:4.8, 100:5.0
//
// Heading Noise Information :
// ---------------------------
//  HEADING_SIGMA = 0.1
//  HEADING_Mu = 0.005
//  HEADING_SATURATION = 1
//
// Water Depth Information :
// -------------------------
//  Water depth: 58

bool USM_MOOSApp::buildReport()
{
  NodeRecord record = m_model.getNodeRecord();
  double nav_x   = record.getX();
  double nav_y   = record.getY();
  double pitch   = record.getPitch();
  double nav_alt = m_model.getWaterDepth() - record.getDepth();
  if(nav_alt < 0)
    nav_alt = 0;

  double nav_lat, nav_lon;
  if(m_geo_ok) {
#ifdef USE_UTM
    m_geodesy.UTM2LatLong(nav_x, nav_y, nav_lat, nav_lon);
#endif
 }

  string datum_lat = doubleToStringX(m_geodesy.GetOriginLatitude(),9);
  string datum_lon = doubleToStringX(m_geodesy.GetOriginLongitude(),9);
  
  m_msgs << "Datum: " + datum_lat + "," + datum_lon;
  m_msgs << endl << endl;
  // Part 1: Pose Information ===========================================
  ACTable actab(4,1);
  actab << "Start | Pose | Current | Pose (NAV)";
  actab.addHeaderLines();
  actab.setColumnPadStr(1, "   "); // Pad w/ extra blanks between cols 1&2
  actab.setColumnPadStr(3, "   "); // Pad w/ extra blanks between cols 3&4
  actab.setColumnJustify(0, "right");
  actab.setColumnJustify(2, "right");
  actab.setColumnJustify(4, "right");

  actab << "Heading:" << m_start_nav_hdg;
  actab << "Heading:" << doubleToStringX(record.getHeading(),1);

  actab << "Speed:" << m_start_nav_spd;
  actab << "Speed:" << doubleToStringX(record.getSpeed(),2);

  actab << "Pitch:" << m_start_nav_ptc;
  actab << "Pitch:" << doubleToStringX(record.getPitch(),2);

  actab << "Depth:" << m_start_nav_dep;
  actab << "Depth:" << doubleToStringX(record.getDepth(),1);

  actab << "Alt:" << m_start_nav_alt;
  actab << "Alt:" << doubleToStringX(nav_alt,1);

  actab << "(X,Y):" << m_start_nav_x +","+ m_start_nav_y;
  actab << "(X,Y):" << doubleToStringX(nav_x,2) + "," + doubleToStringX(nav_y,2); 

  actab << "Lat:" << m_start_nav_lat;
  actab << "Lat:" << doubleToStringX(nav_lat,8);

  actab << "Lon:" << m_start_nav_lon;
  actab << "Lon:" << doubleToStringX(nav_lon,8);

  m_msgs << actab.getFormattedString();

  // Part 3: External Drift Info =======================================
  m_msgs << endl;
  m_msgs << endl;
  actab = ACTable(7,1);
  actab << "Ext Drift | X | Y | Z | Mag | Ang | Rot.";
  actab.addHeaderLines();
  actab.setColumnJustify(0, "right");
  actab.setColumnPadStr(3, "  |  ");
  actab.setColumnPadStr(5, "  |  ");
  actab.setColumnPadStr(6, "  |  ");

  string drift_x   = doubleToStringX(m_model.getDriftX(),3);
  string drift_y   = doubleToStringX(m_model.getDriftY(),3);
  string drift_z   = doubleToStringX(m_model.getDriftZ(),3);
  string drift_mag = doubleToStringX(m_model.getDriftMag(),4);
  string drift_ang = doubleToStringX(m_model.getDriftAng(),4);
  if(drift_mag == "0")
    drift_ang = "0";
  string drift_srcs = "n/a";
  if(m_srcs_drift.size() != 0)
    drift_srcs = setToString(m_srcs_drift);
  actab << "Present" << drift_x << drift_y << drift_z << drift_mag << drift_ang << "None";
  m_msgs << actab.getFormattedString();
  m_msgs << endl << endl;

  // Part 4: Speed/Thrust Info =======================================
  m_msgs << "Velocity Information: " << endl;
  m_msgs << "--------------------- " << endl;
  m_msgs << "     DESIRED_SURGE_THRUST=" << doubleToStringX(m_model.getSurgeThrust(),1) << endl;
  m_msgs << endl;
  m_msgs << "     DESIRED_HEADING_THRUST=" << doubleToStringX(m_model.getHeadingThrust(),1) << endl;
  m_msgs << endl;
  m_msgs << "     DESIRED_PITCH_THRUST=" << doubleToStringX(m_model.getPitchThrust(),1) << endl;
  m_msgs << endl;
  
  string dragmap = m_model.getDragMap();

  m_msgs << "     Positive Drag Map: " << dragmap << endl;
  m_msgs << endl;

  // Part 5: Heading Noise Info ===========================
  m_msgs << "Heading Noise Information: " << endl;
  m_msgs << "-------------------------- " << endl;
  m_msgs << "     HEADING_SIGMA=" << doubleToStringX(m_model.getHeadingSigma(),3) << endl;
  m_msgs << endl;
  m_msgs << "     HEADING_MU=" << doubleToStringX(m_model.getHeadingMu(),3) << endl;
  m_msgs << endl;
  m_msgs << "     HEADING_SATURATION=" << doubleToStringX(m_model.getHeadingSaturation(),3) << endl;
  m_msgs << endl;
  
  // Part 6: Water Depth Change Info ===========================
  m_msgs << "Water Depth Information: " << endl;
  m_msgs << "------------------------ " << endl;
  m_msgs << "     Water Depth: " << doubleToStringX(m_model.getWaterDepth(),2) << endl;
  m_msgs << endl;

  return(true);
}


