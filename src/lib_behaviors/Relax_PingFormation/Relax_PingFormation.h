/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Relax_PingFormation.h                           */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Relax_PingFormation_HEADER
#define Relax_PingFormation_HEADER

  #include "OneContact_PingFormation/OneContact_PingFormation.h"

  class IvPDomain;
  class Relax_PingFormation : public OneContact_PingFormation
  {
    public:
      Relax_PingFormation(IvPDomain);
      ~Relax_PingFormation() {};
      bool          setParam(std::string, std::string);
      IvPFunction*  onRunState();
    
    private: // Configuration parameters
      double        m_range;
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
    { return new Relax_PingFormation(domain); }
  }

#endif