/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: TrailiSBL_PingFormation.h                       */
/*    DATE: June 2014                                       */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef TrailiSBL_PingFormation_HEADER
#define TrailiSBL_PingFormation_HEADER

  #include "OneContact_PingFormation/OneContact_PingFormation.h"

  class IvPDomain;
  class TrailiSBL_PingFormation : public OneContact_PingFormation
  {
    public:
      TrailiSBL_PingFormation(IvPDomain);
      ~TrailiSBL_PingFormation() {};
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
    { return new TrailiSBL_PingFormation(domain); }
  }

#endif