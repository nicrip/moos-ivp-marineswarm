/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Modification of ContactsPing_Formation by Simon Rohou */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ManageAcousticPing.cpp                      */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_ManageAcousticPing.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_ManageAcousticPing::BHV_ManageAcousticPing(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "BHV_ManageAcousticPing");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Initialize config variables
  m_ping_filter_timeout = 120.;
  m_contact_timeout = 120.;
  m_display_filtered_contact = false;
  m_display_unfiltered_contact = false;

  // Initialize state variables
  m_nav_heading = 0.;
  m_nav_depth = 0.;
  m_nav_x = 0.;
  m_nav_y = 0.;
  m_sound_speed = 1500.;
  m_last_ping_id = -1;
  m_num_ping_failures = 0;
  m_num_ping_updates = 0;

  postMessage("MANAGE_ACOUSTIC_PING_SOUND_SPEED", m_sound_speed);

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_DEPTH, NAV_HEADING, PING_REPORT");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_ManageAcousticPing::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  bool non_neg_number = (isNumber(val) && (double_val >= 0));

  if ((param == "ping_filter_timeout") && non_neg_number) {
    m_ping_filter_timeout = double_val;
    return(true);
  } else if ((param == "contact_timeout") && non_neg_number) {
    m_contact_timeout = double_val;
    return(true);
  } else if ((param == "display_filtered_contact")) {
    return(setBooleanOnString(m_display_filtered_contact, val));
  } else if ((param == "display_unfiltered_contact")) {
    return(setBooleanOnString(m_display_unfiltered_contact, val));
  } else if ((param == "sound_speed") && non_neg_number) {
    m_sound_speed = double_val;
    return(true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_ManageAcousticPing::onSetParamComplete()
{
  for (map<string,PingContact>::iterator contact = m_contact_whitelist.begin(); contact != m_contact_whitelist.end(); ++contact) {
    contact->second.GetAbsolutePointSupposed()->set_label(contact->first + "_pos_by_" + toupper(m_us_name));
    contact->second.GetAbsolutePointSupposed()->set_active("false");
    contact->second.GetAbsoluteFilteredPointSupposed()->set_label(contact->first + "_filt_pos_by_" + toupper(m_us_name));
    contact->second.GetAbsoluteFilteredPointSupposed()->set_active("false");
  }
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_ManageAcousticPing::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_ManageAcousticPing::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_ManageAcousticPing::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_ManageAcousticPing::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_ManageAcousticPing::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_ManageAcousticPing::onRunToIdleState()
{
  postErasablePoints();
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_ManageAcousticPing::onRunState()
{
  checkContactValidityAndPostViewablePoints();

  return NULL;
}

//---------------------------------------------------------------
// Procedure: updateInfoIn()
//   Purpose: update the info from MOOSDB vars.

bool BHV_ManageAcousticPing::updateInfoIn()
{
  bool ok1, ok2;

  m_nav_x = getBufferDoubleVal("NAV_X", ok1);
  m_nav_y = getBufferDoubleVal("NAV_Y", ok2);
  if (!ok1 || !ok2) {
    postEMessage("No NAV_X or NAV_Y info in info_buffer.");
    return(false);
  }

  m_nav_heading = getBufferDoubleVal("NAV_HEADING", ok1);
  if (!ok1) {
    postEMessage("No NAV_HEADING info in info_buffer.");
    return(false);
  }

  m_nav_depth = getBufferDoubleVal("NAV_DEPTH", ok1);
  if (!ok1) {
    //postWMessage("No NAV_DEPTH info in info_buffer.");
    //return(false);
  }

  if (m_contact_whitelist.size() == 0)
  {
    postEMessage("Contact whitelist is empty - no contact pings are being managed (maybe add valid contacts to the whitelist?).");
    return(false);
  }

  m_ping_report = getBufferStringVal("PING_REPORT", ok1); //ONLY GETS LATEST PING - MIGHT HAVE MULTIPLE PINGS IN THE MEANTIME!!!
  if (!ok1) { // used only to check that this variable has been published at least once
    postWMessage("Waiting for a PING_REPORT from a valid contact.");
    return(false);
  }

  m_ping_report_vector = getBufferStringVector("PING_REPORT", ok1);

  for(vector<string>::iterator it = m_ping_report_vector.begin(); it != m_ping_report_vector.end(); ++it) {
    m_ping_report = *it;
    MOOSValFromString(m_ping_name, m_ping_report, "name");
    MOOSValFromString(m_curr_ping_id, m_ping_report, "ping_id");
    MOOSValFromString(m_ping_heading, m_ping_report, "sender_heading");
    if (m_curr_ping_id != m_last_ping_id) { // a new contact ping
      m_last_ping_id = m_curr_ping_id;
      if(m_contact_whitelist.count(tolower(m_ping_name)) != 0) { // the contact is on the whitelist - handle its ping
        if (!m_contact_whitelist[tolower(m_ping_name)].UpdatePositionFromPingRecord(m_ping_report, m_sound_speed, m_ping_filter_timeout, m_nav_x, m_nav_y)) { // failed to extract the contact position
          m_num_ping_failures++;
          postEMessage("Failed to extract " + m_ping_name + " position from its PING_REPORT.");
          //return(false);
        } else {
          m_contact_whitelist[tolower(m_ping_name)].SetContactHeading(m_ping_heading);
          m_num_ping_updates++;
        }
      }
    }
  }

  postMessage("MANAGE_ACOUSTIC_PING_NUM_CONTACTS", m_contact_whitelist.size());
  postMessage("MANAGE_ACOUSTIC_PING_NUM_SUCCESSES", m_num_ping_updates);
  postMessage("MANAGE_ACOUSTIC_PING_NUM_FAILURES", m_num_ping_failures);

  return(true);
}

//---------------------------------------------------------------
// Procedure: postViewablePoints()
//   Purpose: Display estimated contact positions.

void BHV_ManageAcousticPing::checkContactValidityAndPostViewablePoints()
{
  for (map<string,PingContact>::iterator contact = m_contact_whitelist.begin(); contact != m_contact_whitelist.end(); ++contact) {
    contact->second.UpdateExpired(m_contact_timeout); // update expiry bool to check if we have reached the contact timeout
    if (m_display_unfiltered_contact) {
      if (contact->second.GetExpired()) {
        contact->second.GetAbsolutePointSupposed()->set_active(false);
      } else {
        contact->second.GetAbsolutePointSupposed()->set_active(true);
      }
      postMessage("VIEW_POINT", contact->second.GetAbsolutePointSupposed()->get_spec());
    }

    if(m_display_filtered_contact) {
      if (contact->second.GetExpired()) {
        contact->second.GetAbsoluteFilteredPointSupposed()->set_active(false);
      } else {
        contact->second.GetAbsoluteFilteredPointSupposed()->set_active(true);
      }
      postMessage("VIEW_POINT", contact->second.GetAbsoluteFilteredPointSupposed()->get_spec());
    }
  }
}

//---------------------------------------------------------------
// Procedure: postErasablePoints()
//   Purpose: Erase estimated contact positions.

void BHV_ManageAcousticPing::postErasablePoints()
{
  for (map<string,PingContact>::iterator contact = m_contact_whitelist.begin(); contact != m_contact_whitelist.end(); ++contact) {
    if (m_display_unfiltered_contact) {
      contact->second.GetAbsolutePointSupposed()->set_active(false);
      postMessage("VIEW_POINT", contact->second.GetAbsolutePointSupposed()->get_spec());
    }

    if(m_display_filtered_contact) {
      contact->second.GetAbsoluteFilteredPointSupposed()->set_active(false);
      postMessage("VIEW_POINT", contact->second.GetAbsoluteFilteredPointSupposed()->get_spec());
    }
  }
}

//---------------------------------------------------------------
// Procedure: addContactToWhiteList()
//   Purpose: Add a contact to the contact whitelist, allowing us to handle pings from this contact.

bool BHV_ManageAcousticPing::addContactToWhiteList(string name)
{
  if (tolower(name) != "") {
    m_contact_whitelist[tolower(name)]; // invokes default contact constructor
    m_contact_whitelist[tolower(name)].SetName(tolower(name));
    m_contact_whitelist[tolower(name)].GetAbsolutePointSupposed()->set_label(tolower(name) + "_pos_by_" + toupper(m_us_name));
    m_contact_whitelist[tolower(name)].GetAbsolutePointSupposed()->set_active("false");
    m_contact_whitelist[tolower(name)].GetAbsolutePointSupposed()->set_color("label","pink");
    m_contact_whitelist[tolower(name)].GetAbsolutePointSupposed()->set_color("vertex","pink");
    m_contact_whitelist[tolower(name)].GetAbsoluteFilteredPointSupposed()->set_label(tolower(name) + "_filt_pos_by_" + toupper(m_us_name));
    m_contact_whitelist[tolower(name)].GetAbsoluteFilteredPointSupposed()->set_active("false");
    m_contact_whitelist[tolower(name)].GetAbsoluteFilteredPointSupposed()->set_color("label","pink");
    m_contact_whitelist[tolower(name)].GetAbsoluteFilteredPointSupposed()->set_color("vertex","pink");
    return(true);
  }

  return(false);
}

//---------------------------------------------------------------
// Procedure: removeContactFromWhiteList()
//   Purpose: Remove a contact from the contact whitelist.

void BHV_ManageAcousticPing::removeContactFromWhiteList(string name)
{
  m_contact_whitelist.erase(name);
}

//---------------------------------------------------------------
// Procedure: getContact()
//   Purpose: Get a contact from the contact whitelist.

PingContact* BHV_ManageAcousticPing::getContact(std::string name)
{
  if (m_contact_whitelist.count(name) != 0) {
    return &(m_contact_whitelist[name]);
  }

  return NULL;
}
