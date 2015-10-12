/* ******************************************************** */
/* ORGN: MOOSSafir - CGG (Massy - France)                   */
/* FILE: VirtualStructure_MapPingFormation.h                */
/* DATE: July 2014                                          */
/* AUTH: Nathan Vandervaeren                                */
/* ******************************************************** */
 
#ifndef VirtualStructure_MapPingFormation_HEADER
#define VirtualStructure_MapPingFormation_HEADER

  #include <armadillo>
  #include "Map_PingFormation/Map_PingFormation.h"

  class IvPDomain;
  class VirtualStructure_MapPingFormation : public Map_PingFormation
  {
    public:
      VirtualStructure_MapPingFormation(IvPDomain);
      ~VirtualStructure_MapPingFormation() {};
      bool setParam(std::string, std::string);
      IvPFunction* onRunState();
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
    { return new VirtualStructure_MapPingFormation(domain); }
  }

#endif