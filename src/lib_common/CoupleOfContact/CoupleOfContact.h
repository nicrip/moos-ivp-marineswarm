/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: CoupleOfContact.h                               */
/*    DATE: april 2014                                      */
/* ******************************************************** */

#ifndef CoupleOfContact_HEADER
#define CoupleOfContact_HEADER

#include <string>
#include "XYPoint.h"
#include "Ping/PingContact.h"

class CoupleOfContact
{
	public:
		CoupleOfContact(std::string name);
    ~CoupleOfContact() {};
    
    void setName(std::string name);
    void setContact1(PingContact* ping_contact);
    void setContact2(PingContact* ping_contact);
    PingContact* getContact1();
    PingContact* getContact2();
    void setDistFromMidpoint(double dist_from_midpoint);
    void setWeight(double weight);
    bool IsInitialized();
    
    XYPoint getTargetPoint();
    std::string getName();
    double getWeight();
    
  private:
    std::string m_name;
    PingContact* m_contact_1;
    PingContact* m_contact_2;
    double m_dist_from_midpoint;
    double m_weight;
};

#endif 