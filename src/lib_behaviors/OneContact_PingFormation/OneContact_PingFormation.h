/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: OneContact_PingFormation.h                      */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef OneContact_PingFormation_HEADER
#define OneContact_PingFormation_HEADER

  #include "ContactsPing_Formation/ContactsPing_Formation.h"

  class IvPDomain;
  class OneContact_PingFormation : public ContactsPing_Formation
  {
    public:
      OneContact_PingFormation(IvPDomain);
      ~OneContact_PingFormation() {};
      
      bool        setParam(std::string, std::string);
      PingContact getContact();
  };

#endif