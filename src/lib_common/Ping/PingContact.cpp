/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: PingContact.cpp                                 */
/*    DATE: april 2014                                      */
/* ******************************************************** */

#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include "AngleUtils.h"
#include "GeomUtils.h"
#include "PingContact.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "CoordinateSystem/SphericalCoordinateSystem.h"
#include <limits>

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

PingContact::PingContact()
{
  m_name = "?";

  m_raw_relative_x = 0.;
  m_raw_relative_y = 0.;
  m_raw_relative_z = 0.;

  m_filtered_relative_x = 0.;
  m_filtered_relative_y = 0.;
  m_filtered_relative_z = 0.;

  m_absolute_point_supposed.set_vertex(0, 0, 0); // only used for display
  m_absolute_filtered_point_supposed.set_vertex(0, 0, 0); // only used for display
  m_contact_heading = 0.;

  //m_time_last_update = MOOSTime();
  m_time_last_update = 0.;

  m_init = false;

  m_debug_string = new ostringstream();
  m_debug_string->clear();
  m_debug_string->str("");

  m_expired = true;
}


//-----------------------------------------------------------
// Procedure: GetName

string PingContact::GetName()
{
  return m_name;
}


//-----------------------------------------------------------
// Procedure: SetName

void PingContact::SetName(string name)
{
  m_name = name;
}


//-----------------------------------------------------------
// Procedure: SetContactHeading

void PingContact::SetContactHeading(double heading)
{
  m_contact_heading = heading;
}


//-----------------------------------------------------------
// Procedure: GetRelativeX

double PingContact::GetRelativeX()
{
  return m_filtered_relative_x;
}


//-----------------------------------------------------------
// Procedure: GetRelativeY

double PingContact::GetRelativeY()
{
  return m_filtered_relative_y;
}


//-----------------------------------------------------------
// Procedure: GetRelativeZ

double PingContact::GetRelativeZ()
{
  return m_filtered_relative_z;
}


//-----------------------------------------------------------
// Procedure: GetContactHeading

double PingContact::GetContactHeading()
{
  return m_contact_heading;
}


//-----------------------------------------------------------
// Procedure: GetAbsolutePointSupposed

XYPoint* PingContact::GetAbsolutePointSupposed()
{
  return &m_absolute_point_supposed;
}


//-----------------------------------------------------------
// Procedure: GetAbsoluteFilteredPointSupposed

XYPoint* PingContact::GetAbsoluteFilteredPointSupposed()
{
  return &m_absolute_filtered_point_supposed;
}


//-----------------------------------------------------------
// Procedure: UpdatePositionFromPingRecord

bool PingContact::UpdatePositionFromPingRecord(string ping_record, double sound_speed, double expiration_delay, double m_nav_x, double m_nav_y)
{
  // measure of the sender relative position
  double dt, rho_raw, rho_filtered, theta_raw, theta_filtered, phi_raw,  phi_filtered;
  MOOSValFromString(dt, ping_record, "dt");
  MOOSValFromString(theta_raw, ping_record, "theta");
  MOOSValFromString(phi_raw, ping_record, "phi");
  rho_raw = dt * sound_speed;

  // Filtering:
  rho_filtered = rho_raw;
  theta_filtered = theta_raw;
  phi_filtered = phi_raw;
  FilterSphericalCoordinates(rho_filtered, theta_filtered, phi_filtered, expiration_delay);

  bool result_raw = SphericalCoordinateSystem::Spherical2Cartesian(rho_raw, theta_raw, phi_raw,
                                                                   m_raw_relative_x, m_raw_relative_y, m_raw_relative_z);
  bool result_filtered = SphericalCoordinateSystem::Spherical2Cartesian(rho_filtered, theta_filtered, phi_filtered,
                                                                        m_filtered_relative_x, m_filtered_relative_y, m_filtered_relative_z);

  m_init = m_init || (result_raw && result_filtered);

  UpdateAbsolutePointSupposed(m_nav_x, m_nav_y);
  UpdateAbsoluteFilteredPointSupposed(m_nav_x, m_nav_y);

  m_time_last_update = MOOSTime();
  m_expired = false;

  return (result_raw && result_filtered);
}


//-----------------------------------------------------------
// Procedure: GetCurrentFilterSize

int PingContact::GetCurrentFilterSize()
{
  return m_list_previous_spherical_coordinates.size();
}


//-----------------------------------------------------------
// Procedure: FilterSphericalCoordinates

void PingContact::FilterSphericalCoordinates(double& rho, double& theta, double& phi, double expiration_delay)
{
  //m_debug_string->clear();
  //m_debug_string->str("");

  if(expiration_delay == 0)
    return;

  map<string, double> spherical_coordinates;
  spherical_coordinates["rho"] = rho;
  spherical_coordinates["theta"] = theta;
  spherical_coordinates["phi"] = phi;

  m_list_previous_spherical_coordinates[MOOSTime()] = spherical_coordinates;

  double sum_weights = 0., sum_rho = 0., sum_sin_theta = 0., sum_cos_theta = 0., sum_sin_phi = 0., sum_cos_phi = 0.;

  for(map<double,map<string,double> >::iterator previous_spherical_coordinates = m_list_previous_spherical_coordinates.begin() ;
      previous_spherical_coordinates != m_list_previous_spherical_coordinates.end() ; )
  {
    double delta_time = MOOSTime() - previous_spherical_coordinates->first;

    //*m_debug_string << "DELTA TIME: " << previous_spherical_coordinates->first << ", " << delta_time << " | ";

    if(delta_time > expiration_delay)
    {
      // deleting inside the loop..
      m_list_previous_spherical_coordinates.erase(previous_spherical_coordinates++);
    }

    else
    {
      double weight = 1. - (delta_time / expiration_delay);

      sum_weights    += weight;
      sum_rho        += weight * previous_spherical_coordinates->second["rho"];

      // Filter on degrees: be careful, mean value of (0, 360) != 180
      sum_sin_theta  += weight * sin(previous_spherical_coordinates->second["theta"]);
      sum_sin_phi    += weight * sin(previous_spherical_coordinates->second["phi"]);
      sum_cos_theta  += weight * cos(previous_spherical_coordinates->second["theta"]);
      sum_cos_phi    += weight * cos(previous_spherical_coordinates->second["phi"]);

      ++previous_spherical_coordinates;
    }
  }

  rho = sum_rho / sum_weights;
  phi = atan2(sum_sin_phi, sum_cos_phi);
  theta = atan2(sum_sin_theta, sum_cos_theta);
  //WAIT! SHOULD WE DIVIDE THESE ANGLES BY WEIGHTS?
}


//-----------------------------------------------------------
// Procedure: lapsing

bool PingContact::lapsing()
{
  return MOOSTime() - m_time_last_update;
}


//-----------------------------------------------------------
// Procedure: IsInitialized

bool PingContact::IsInitialized()
{
  return m_init;
}


//-----------------------------------------------------------
// Procedure: HasExpired

bool PingContact::HasExpired()
{
  return (MOOSTime() - m_time_last_update) > 100;
}


//-----------------------------------------------------------
// Procedure: UpdateExpired

void PingContact::UpdateExpired(double expire_timeout)
{
  if ((MOOSTime() - m_time_last_update) > expire_timeout) {
    m_expired = true;
  } else {
    m_expired = false;
  }
}


//-----------------------------------------------------------
// Procedure: GetExpired

bool PingContact::GetExpired()
{
  return m_expired;
}


//-----------------------------------------------------------
// Procedure: GetExpired

double PingContact::GetTimeLapse()
{
  return (MOOSTime() - m_time_last_update);
}


//-----------------------------------------------------------
// Procedure: UpdateAbsolutePointSupposed

void PingContact::UpdateAbsolutePointSupposed(double nav_x, double nav_y)
{
  m_absolute_point_supposed.set_vertex(nav_x + m_raw_relative_x, nav_y + m_raw_relative_y, 0);
}


//-----------------------------------------------------------
// Procedure: UpdateAbsolutePointSupposed

void PingContact::UpdateAbsoluteFilteredPointSupposed(double nav_x, double nav_y)
{
  m_absolute_filtered_point_supposed.set_vertex(nav_x + m_filtered_relative_x, nav_y + m_filtered_relative_y, 0);
}

string PingContact::GetDebugString()
{
  return m_debug_string->str();
}
