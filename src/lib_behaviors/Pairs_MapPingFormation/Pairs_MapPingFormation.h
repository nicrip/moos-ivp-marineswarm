/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Pairs_MapPingFormation.h                        */
/*    DATE: March 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Pairs_MapPingFormation_HEADER
#define Pairs_MapPingFormation_HEADER

  #include "Ping/PingContact.h"
  #include "Map_PingFormation/Map_PingFormation.h"

  class IvPDomain;
  class Pairs_MapPingFormation : public Map_PingFormation
  {
    public:
      Pairs_MapPingFormation(IvPDomain);
      ~Pairs_MapPingFormation() {};
      bool          setParam(std::string, std::string);
      void          onSetParamComplete();
      IvPFunction*  onRunState();
      void          postViewablePoints();
      void          postErasablePoints();
      void          onRunToIdleState();
      
    private:
      double  m_ping_expiration_distance;
  };

  #ifdef WIN32
    // Windows needs to explicitly specify functions to export from a dll
     #define IVP_EXPORT_FUNCTION __declspec(dllexport) 
  #else
     #define IVP_EXPORT_FUNCTION
  #endif

  extern "C"
  {
    IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
    { return new Pairs_MapPingFormation(domain); }
  }

#endif