/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Trail_PingFormation.h                           */
/*    DATE: June 2014                                       */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Trail_PingFormation_HEADER
#define Trail_PingFormation_HEADER

  #include "OneContact_PingFormation/OneContact_PingFormation.h"

  class IvPDomain;
  class Trail_PingFormation : public OneContact_PingFormation
  {
    public:
      Trail_PingFormation(IvPDomain);
      ~Trail_PingFormation() {};
      bool          setParam(std::string, std::string);
      IvPFunction*  onRunState();
    
    private: // Configuration parameters
      double        m_trail_dx;
      double        m_trail_dy;
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
    { return new Trail_PingFormation(domain); }
  }

#endif