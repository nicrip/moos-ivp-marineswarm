/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: USM_MOOSApp.h                                        */
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

#ifndef USM_MOOSAPP_HEADER
#define USM_MOOSAPP_HEADER

#include <string>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOSGeodesy/MOOSGeodesy.h"
#include "USM_Model.h"

class USM_MOOSApp : public AppCastingMOOSApp
{
public:
  USM_MOOSApp();
  virtual ~USM_MOOSApp() {};

 public: // Standard MOOSApp functions to overload
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool OnStartUp();
  bool Iterate();
  bool OnConnectToServer();

 protected: // Standard AppCastingMOOSApp function to overload
  bool buildReport();

 protected:
  void registerVariables();
  void postNodeRecordUpdate(std::string, const NodeRecord&);
  void cacheStartingInfo();

protected:
  std::string  m_sim_prefix;
  USM_Model    m_model;
  unsigned int m_reset_count;

  CMOOSGeodesy m_geodesy;
  bool         m_geo_ok;

  // A cache of starting info to facilitate generation of reports.
  std::string m_start_nav_x;
  std::string m_start_nav_y;
  std::string m_start_nav_hdg;
  std::string m_start_nav_spd;
  std::string m_start_nav_ptc;
  std::string m_start_nav_dep;
  std::string m_start_nav_alt;
  std::string m_start_nav_lat;
  std::string m_start_nav_lon;
  std::string m_start_drift_x;
  std::string m_start_drift_y;
  std::string m_start_drift_z;
  std::string m_start_drift_mag;
  std::string m_start_drift_ang;

  std::set<std::string> m_srcs_drift;

  double m_last_report;
  double m_report_interval;
};
#endif




