/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: MarinePID.h                                          */
/*    DATE: April 10 2006                                        */
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

#ifndef MARINE_PID_HEADER
#define MARINE_PID_HEADER

#include <string>
#include "MOOS/libMOOS/MOOSLib.h"
#include "PIDEngine.h"
#include "SimMap/DragMap.h"

class MarinePID : public CMOOSApp
{
public:
  MarinePID();
  virtual ~MarinePID() {};
  
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();
  void postCharStatus();
  void postAllStop();
  void registerVariables();
  
  bool handleYawSettings();
  bool handleSpeedSettings();
  bool handleDepthSettings();

protected:
  bool     m_has_control;
  bool     m_allow_overide;
  bool     m_allstop_posted;

  double   m_current_heading;
  double   m_current_speed;
  double   m_current_depth;
  double   m_current_pitch;

  double   m_desired_heading;
  double   m_desired_speed;
  double   m_desired_depth;
  double   m_current_surge_thrust;

  double   m_max_pitch_thrust;
  double   m_max_heading_thrust;
  double   m_max_surge_thrust;
  double   m_max_speed;
  double   m_max_pitch;
  double   m_heading_limit;
       
  DragMap      m_drag_map;

  PIDEngine    m_pengine;
  std::string  m_verbose;

  int     m_iteration;
  double  m_start_time;
  bool    m_depth_control;
  bool    m_paused;
  bool    m_spdPID_active;

  double  m_time_of_last_helm_msg;
  double  m_time_of_last_nav_msg;

  double  m_tardy_helm_thresh;
  double  m_tardy_nav_thresh;
};
#endif 


