/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USM_Info.cpp                                         */
/*    DATE: July 6th 2011                                        */
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
 
#include <cstdlib>
#include <iostream>
#include "USM_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;
//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The uSimMarineFreedom_CGG application is a simple 3D vehicle simulator  ");
  blk("  that updates vehicle state, position and trajectory, based on ");
  blk("  the present actuator values and prior vehicle state. Typical  ");
  blk("  usage scenario has a single instance of uSimMarineFreedom_CGG associated");
  blk("  with each simulated vehicle.                                  ");
}


//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: uSimMarineFreedom_CGG file.moos [OPTIONS]                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch uSimMarineFreedom_CGG with the given process name rather     ");
  blk("      than uSimMarineFreedom_CGG.                                         ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of uSimMarineFreedom_CGG.               ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uSimMarineFreedom_CGG Example MOOS Configuration                          ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uSimMarineFreedom_CGG                                     ");
  blk("{                                                               ");
  blk("  AppTick   = 100                                                 ");
  blk("  CommsTick = 100                                                 ");
  blk("                                                                ");
  blk("  start_x       = 0                                             ");
  blk("  start_y       = 0                                             ");
  blk("  start_heading = 0                                             ");
  blk("  start_speed   = 0                                             ");
  blk("  start_depth   = 0                                             ");
  blk("  start_pos     = x=0, y=0, speed=0, heading=0, depth=0         ");
  blk("                                                                ");
  blk("  drift_x       = 0                                             ");
  blk("  drift_y       = 0                                             ");
  blk("  drift_z       = 0                                             ");
  blk("                                                                ");
  blk("  sim_pause            = false ","// or {true}                  ");
  blk("                                                                ");
  blk("  prefix               = NAV_  ","// default is USM_            ");
  blk("                                                                ");
  blk("  max_thrust_force   =   7     // newtons                       ");
  blk("  max_rudder_angle   =   13.6  // degrees                       ");
  blk("  max_elevator_angle =   13.6  // degrees                       ");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uSimMarineFreedom_CGG INTERFACE                                           ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  DESIRED_THRUST     = [-100,100]                               ");
  blk("  DESIRED_RUDDER     = [-100,100]                               ");
  blk("  DESIRED_ELEVATOR   = [-100,100]                               ");
  blk("                                                                ");
  blk("  BUOYANCY_CONTROL   = [-inf,+inf]           // not used        ");
  blk("  BUOYANCY_RATE      = [-inf,+inf] m/s       // not used        ");
  blk("  CURRENT_FIELD      = [true/false]          // not used        ");
  blk("  DRIFT_X/CURRENT_X  = [-inf,+inf] m/s                          ");
  blk("  DRIFT_Y/CURRENT_Y  = [-inf,+inf] m/s                          ");
  blk("  DRIFT_VECTOR       = [0,360),[0,+inf]                         ");
  blk("  DRIFT_VECTOR_ADD   = [-inf,+inf]                              ");
  blk("  DRIFT_VECTOR_MULT  = [-inf,+inf]                              ");
  blk("  ROTATE_SPEED       = [0,inf] m/s           // not used        ");
  blk("  TRIM_CONTROL       = [-inf,+inf]           // not used        ");
  blk("  WATER_DEPTH        = [0,+inf]                                 ");
  blk("                                                                ");
  blk("  USM_RESET            (value not read)                         ");
  blk("  USM_SIM_PAUSED     = [true/false]                             ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  BUOYANCY_REPORT         = status=2,error=0,buoyancy=0.0       ");
  blk("  TRIM_CONTROL            = status=2,error=0,trim_pitch=0.0,    ");
  blk("                            trim_roll=0.0                       ");
  blk("  USM_ALTITUDE            = 100                                 ");
  blk("  USM_DEPTH               = 45                                  ");
  blk("  USM_DRIFT_SUMMARY       = ang=90, mag=1.5, xmag=90, ymag=0    ");
  blk("  USM_HEADING             = 197                                 ");
  blk("  USM_HEADING_OVER_GROUND = 192                                 ");
  blk("  USM_LAT                 = 42.1293844                          ");
  blk("  USM_LONG                = -73.2398311                         ");
  blk("  USM_SPEED               = 1.33                                ");
  blk("  USM_SURGE_SPEED         = 1.26  // for use in uSimConsumption ");
  blk("  USM_SPEED_OVER_GROUND   = 2.09                                ");
  blk("  USM_X                   = 34.9                                ");
  blk("  USM_Y                   = 442.5                               ");
  blk("  USM_Z                   = -200                                ");
  blk("  USM_YAW                 = 3.14     // theta in radians        ");
  blk("  USM_PITCH               = 0.23     // psi in radians          ");
  blk("  USM_PHI                 = 197      // in degrees              ");
  blk("  USM_THETA               = 197      // in degrees              ");
  blk("  USM_PSI                 = 197      // in degrees              ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uSimMarineFreedom_CGG   ", "gpl");
  exit(0);
}


