/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Modification of ContactsPing_Formation by Simon Rohou */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ManageAcousticPing.h                        */
/*    DATE:                                                 */
/************************************************************/

#ifndef ManageAcousticPing_HEADER
#define ManageAcousticPing_HEADER

#include <string>
#include "IvPBehavior.h"
#include "Ping/PingContact.h"
#include "MOOS/libMOOS/MOOSLib.h"

class BHV_ManageAcousticPing : public virtual IvPBehavior {
public:
  BHV_ManageAcousticPing(IvPDomain);
  ~BHV_ManageAcousticPing() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  bool          updateInfoIn();
  void          checkContactValidityAndPostViewablePoints();
  void          postErasablePoints();
  bool          addContactToWhiteList(std::string name);
  void          removeContactFromWhiteList(std::string name);
  PingContact*  getContact(std::string name);

protected: // Configuration parameters
  double        m_ping_filter_timeout;
  double        m_contact_timeout;
  bool          m_display_filtered_contact;
  bool          m_display_unfiltered_contact;
  double        m_sound_speed;

protected: // State variables
  double        m_nav_heading;
  double        m_nav_depth;
  double        m_nav_x;
  double        m_nav_y;
  std::string   m_ping_report;
  std::vector<std::string> m_ping_report_vector;
  std::string   m_ping_name;
  double        m_ping_heading;
  std::map<std::string, PingContact> m_contact_whitelist;
  int           m_curr_ping_id;
  int           m_last_ping_id;
  int           m_num_ping_failures;
  int           m_num_ping_updates;
};

#endif
