/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: SimEngine.h                                          */
/*    DATE: Nov 19th 2006 (as separate class under MVC paradigm  */
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

#ifndef SIM_ENGINE_HEADER
#define SIM_ENGINE_HEADER

#include "NodeRecord.h"
#include "SimMap/DragMap.h"

class SimEngine
{
public:
  SimEngine() {};
  ~SimEngine() {};
  
public:
  void propagatePosition(NodeRecord& record, 
                         double delta_time, 
                         double prior_heading,      // degrees
                         double prior_speed, 
                         double drift_x, 
                         double drift_y);
                         
  void propagateDepth(NodeRecord& record,
                      double delta_time,
                      double drift_z);
  
  void propagateSpeed(NodeRecord& record, 
                      double delta_time, 
                      double surge_thrust,          // 0:100
                      double mass,                  // kg
                      double max_speed,
                      DragMap drag_map);
            
  void propagateHeading(NodeRecord& record, 
                        double delta_time,
                        double heading_thrust,      // -100:100
                        double max_rotate_speed,    // degrees per sec
                        double& heading_rate_noise, // degrees per sec
                        double sigma,
                        double mu,
                        double saturation);
                        
  void propagatePitch(NodeRecord& record, 
                      double delta_time, 
                      double pitch_thrust,          // -100:100
                      double max_rotate_speed);
      
};

#endif


