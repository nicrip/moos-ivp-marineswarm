/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Map_PingFormation.h                             */
/*    DATE: June 2014                                       */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Map_PingFormation_HEADER
#define Map_PingFormation_HEADER

  #include "Ping/PingContact.h"
  #include "ContactsPing_Formation/ContactsPing_Formation.h"

  class IvPDomain;
  class Map_PingFormation : public ContactsPing_Formation
  {
    public:
      Map_PingFormation(IvPDomain);
      ~Map_PingFormation() {};
      bool          setParam(std::string, std::string);
      void          onSetParamComplete();
      IvPFunction*  onRunState();
      void          postViewablePoints();
      void          postErasablePoints();
      void          onRunToIdleState();
      
    protected:
      std::map<std::string,XYPoint> m_map_swarm_elements_position;
  };

#endif