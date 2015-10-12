/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USM_Model.h                                          */
/*    DATE: Nov 19th 2006 (as separate class under MVC paradigm) */
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

#ifndef USM_MODEL_HEADER
#define USM_MODEL_HEADER

#include <string>
#include "NodeRecord.h"
#include "MBTimer.h"
#include "SimEngine.h"

class USM_Model
{
public:
  USM_Model();
  virtual  ~USM_Model() {};

  bool   propagate(double time);
  void   resetTime(double time);
  
  // Setters
  bool   setParam(std::string, double);

  void   setSurgeThrust(double v)   {m_surge_thrust = v;};
  void   setHeadingThrust(double v) {m_heading_thrust = v;};
  void   setPitchThrust(double v)   {m_pitch_thrust = v;};
  void   setDriftX(double v)        {m_drift_x = v;};
  void   setDriftY(double v)        {m_drift_y = v;};
  void   setDriftZ(double v)        {m_drift_z = v;};
  void   setWaterDepth(double v)    {m_water_depth = v;};
  
  bool   handleDragMap(std::string value);

  void   setPaused(bool); 

  bool   initPosition(const std::string&);

  // Getters
  double     getSurgeThrust() const       {return(m_surge_thrust);};
  double     getHeadingThrust() const     {return(m_heading_thrust);};
  double     getPitchThrust() const       {return(m_pitch_thrust);};
  double     getDriftX() const            {return(m_drift_x);};
  double     getDriftY() const            {return(m_drift_y);};
  double     getDriftZ() const            {return(m_drift_z);};
  double     getDriftMag() const;
  double     getDriftAng() const;
  double     getWaterDepth() const        {return(m_water_depth);};
  double     getHeadingRateNoise() const  {return(m_heading_rate_noise);};
  double     getHeadingSigma() const      {return(m_heading_sigma);};
  double     getHeadingMu() const         {return(m_heading_mu);};
  double     getHeadingSaturation() const {return(m_heading_saturation);};
  
  std::string     getDragMap() const     {return(m_drag_map.getMapPos());};

  NodeRecord getNodeRecord() const   {return(m_record);};

  std::string getDriftSummary();

 protected:
  void       propagateNodeRecord(NodeRecord&, double delta_time);

 protected:
  bool       m_paused;
  
  double     m_surge_thrust;        // 0:100
  double     m_heading_thrust;      // -100:100
  double     m_pitch_thrust;        // -100:100
  
  double     m_mass;                // kg // including added-mass
  double     m_max_speed;
  double     m_max_rotate_speed;
  DragMap    m_drag_map;
  
  double     m_water_depth;
  
  double     m_drift_x;           // meters per sec
  double     m_drift_y;           // meters per sec
  double     m_drift_z;           // meters per sec
  
  NodeRecord m_record;       // NAV_X, NAV_Y, etc.        
  
  // for heading noise
  double     m_heading_rate_noise;  // accumulation of heading_rate_noise
  double     m_heading_sigma;
  double     m_heading_mu;
  double     m_heading_saturation;   

  MBTimer    m_pause_timer;
  SimEngine  m_sim_engine;
};
#endif


