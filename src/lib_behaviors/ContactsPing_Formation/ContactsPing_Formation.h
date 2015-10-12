/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: ContactsPing_Formation.h                        */
/*    DATE: March 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef ContactsPing_Formation_HEADER
#define ContactsPing_Formation_HEADER

  #include "Ping/PingContact.h"
  #include "Formation_Behavior/Formation_Behavior.h"

  class IvPDomain;
  class ContactsPing_Formation : public Formation_Behavior
  {
    public:
      ContactsPing_Formation(IvPDomain);
      ~ContactsPing_Formation() {};
      
      bool          setParam(std::string, std::string);
      bool          addContactToWhiteList(string param_val);
      void          onSetParamComplete();

    protected:
      bool          updatePlatformInfo();
      bool          ExtractContactPositionFromPingRecord(string ping_record);
      IvPFunction*  onRunState();

      void          postViewablePoints();
      void          postErasablePoints();
      PingContact*  getContact(std::string contact_name);
      void          displayContactSegment(std::string contact_name, bool active);
      
      // Contacts
      std::map<std::string,PingContact> m_map_contacts;
      bool                      m_display_contact_position_supposed;
      bool                      m_display_filtered_contact_position_supposed;
      double                    m_ping_expiration_delay;
      
      // Environment
      double        m_sound_speed;
      double        m_water_temperature;
      double        m_water_salinity;
      double        m_time_on_leg;
      
      int           m_last_ping_id;
      int           m_n_ping_fails;
      int           m_n_ping_updates;
  };

#endif