/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Isosceles_PingFormation.h                       */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Isosceles_PingFormation_HEADER
#define Isosceles_PingFormation_HEADER

  #include <list>
  #include "ContactsPing_Formation/ContactsPing_Formation.h"
  #include "CoupleOfContact/CoupleOfContact.h"

  class IvPDomain;
  class Isosceles_PingFormation : public ContactsPing_Formation
  {
    public:
      Isosceles_PingFormation(IvPDomain);
      ~Isosceles_PingFormation() {};
      IvPFunction*  onRunState();
      bool          setParam(std::string, std::string);
      void          onSetParamComplete();

    protected:
      void          updateCentroidTargetPoint();

    private: // Configuration parameters
      std::list<CoupleOfContact*> m_list_couple_of_contact;
      bool m_display_sub_target_points;
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
    { return new Isosceles_PingFormation(domain); }
  }

#endif