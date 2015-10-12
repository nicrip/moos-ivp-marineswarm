/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: PingContact.h                                   */
/*    DATE: april 2014                                      */
/* ******************************************************** */

#ifndef PingContact_HEADER
#define PingContact_HEADER

#include <map>
#include <list>
#include <string>
#include <sstream>
#include "XYSegList.h"

class PingContact
{
	public:
		PingContact();
		~PingContact() { };
		std::string GetName();
		void SetName(std::string name);
		void SetContactHeading(double heading);
    double GetRelativeX();
    double GetRelativeY();
    double GetRelativeZ();
    double GetContactHeading();
    XYPoint* GetAbsolutePointSupposed();
    XYPoint* GetAbsoluteFilteredPointSupposed();
    bool UpdatePositionFromPingRecord(std::string ping_record, double sound_speed, double expiration_delay, double nav_x, double nav_y);
    void UpdateAbsoluteFilteredPointSupposed(double nav_x, double nav_y);
    void UpdateAbsolutePointSupposed(double nav_x, double nav_y);
    void FilterSphericalCoordinates(double& rho, double& theta, double& phi, double expiration_delay);
    int GetCurrentFilterSize();
    bool IsInitialized();
    bool HasExpired();
    void UpdateExpired(double expire_timeout);
    bool GetExpired();
    double GetTimeLapse();
    bool lapsing();
    std::string GetDebugString();

	protected:
    bool          m_init;
    std::string   m_name;
    double        m_time_last_update;
    double        m_raw_relative_x;
    double        m_raw_relative_y;
    double        m_raw_relative_z;
    double        m_filtered_relative_x;
    double        m_filtered_relative_y;
    double        m_filtered_relative_z;
    XYPoint       m_absolute_point_supposed;
    XYPoint       m_absolute_filtered_point_supposed;
    std::map<double, std::map<std::string, double> > m_list_previous_spherical_coordinates;
    bool          m_expired;

	private:
    double m_contact_heading;
    std::ostringstream* m_debug_string;
};

#endif
