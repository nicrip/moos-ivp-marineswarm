/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: ContactsPing_Formation.cpp                      */
/*    DATE: March 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */

#include "ContactsPing_Formation.h"
#include "Acoustic/SoundPropagationInWater.h"

using namespace std;


//-----------------------------------------------------------
// Procedure: Constructor

ContactsPing_Formation::ContactsPing_Formation(IvPDomain gdomain) : Formation_Behavior(gdomain)
{
  setParam("name", "ContactsPing_Formation");

  m_sound_speed                       = 1500.;
  m_water_temperature                 = 12.;
  m_water_salinity                    = 35.;
  m_ping_expiration_delay             = 15.;

  m_display_contact_position_supposed = false;
  m_display_filtered_contact_position_supposed = false;

  postMessage("SOUND_SPEED", m_sound_speed);

  m_last_ping_id = -1;
  m_n_ping_updates = 0;
  m_n_ping_fails = 0;

  addInfoVars("PING_REPORT");
}


//-----------------------------------------------------------
// Procedure: updatePlatformInfo

bool ContactsPing_Formation::updatePlatformInfo()
{
  if(!Formation_Behavior::updatePlatformInfo())
    return false;

  bool ok = true;

  if(m_map_contacts.size() == 0)
  {
    string e_message = "updateInfo: empty contacts white list.";
    postEMessage(e_message);
    cout << e_message << endl;
    return false;
  }

  int ping_id;
  string node_report, name;
  node_report = getBufferStringVal("PING_REPORT", ok);

  if(!ok)
  {
    string e_message = "updateInfo: waiting for a node report";
    cout << e_message << endl;
    return false;
  }

  MOOSValFromString(name, node_report, "name");
  MOOSValFromString(ping_id, node_report, "ping_id");

  if(ping_id != m_last_ping_id)
  {
    m_last_ping_id = ping_id;

    if(m_map_contacts.count(name) != 0) // if the contact is allowed
    {
      if(!ExtractContactPositionFromPingRecord(node_report))
      {
        m_n_ping_fails ++;
        string e_message = "updateInfo: error while extracting contact position from ping record.";
        postEMessage(e_message);
        cout << e_message << endl;
        return false;
      }

      else
      {
        m_n_ping_updates ++;
      }
    }
  }

  postMessage("NB_CONTACTS", m_map_contacts.size());
  postMessage("PING_UPDATES", m_n_ping_updates);
  postMessage("PING_FAILS", m_n_ping_fails);

  return true;
}


//---------------------------------------------------------
// Procedure: ExtractNodeRecordFromPingRecordMsg

bool ContactsPing_Formation::ExtractContactPositionFromPingRecord(string ping_record)
{
  string sender_name;
  double sender_heading;
  MOOSValFromString(sender_name, ping_record, "name");
  MOOSValFromString(sender_heading, ping_record, "sender_heading");
  sender_name = tolower(sender_name);

  if(m_sound_speed == -1) // if sound speed not defined
  {
    m_sound_speed = SoundPropagationInWater::GetTheoreticalSpeed(
                      m_nav_depth,
                      m_water_temperature,
                      m_water_salinity);
    postMessage("SOUND_SPEED", m_sound_speed);
  }

  if(m_map_contacts.count(sender_name) != 0)
  {
    if(!m_map_contacts[sender_name].UpdatePositionFromPingRecord(ping_record,
                                                                  m_sound_speed,
                                                                  m_ping_expiration_delay,
                                                                  m_nav_x,
                                                                  m_nav_y))
    {
      string e_message = "UpdatePositionFromPingRecord: error.";
      postEMessage(e_message);
      cout << e_message << endl;
      return false;
    }

    m_map_contacts[sender_name].SetContactHeading(sender_heading);
  }

  return true;
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *ContactsPing_Formation::onRunState()
{
  if(!Formation_Behavior::updatePlatformInfo())
    return 0;

  postViewablePoints();
  return Formation_Behavior::onRunState();
}


//-----------------------------------------------------------
// Procedure: setParam

bool ContactsPing_Formation::setParam(string param, string param_val)
{
  if(Formation_Behavior::setParam(param, param_val))
    return true;

  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "ping_expiration_delay")
  {
    m_ping_expiration_delay = dval;
    return true;
  }

  if(param == "display_contact_position_supposed")
    return setBooleanOnString(m_display_contact_position_supposed, param_val);

  if(param == "display_filtered_contact_position_supposed")
    return setBooleanOnString(m_display_filtered_contact_position_supposed, param_val);

  postMessage("PING_EXPIRATION_DELAY", m_ping_expiration_delay);

  return false;
}


//-----------------------------------------------------------
// Procedure: onSetParamComplete

void ContactsPing_Formation::onSetParamComplete()
{
  Formation_Behavior::onSetParamComplete();

  for(map<string,PingContact>::iterator contact = m_map_contacts.begin() ;
      contact != m_map_contacts.end() ;
      ++contact)
  {
    contact->second.GetAbsolutePointSupposed()->set_label(contact->first + "_point_supposed_by_" + toupper(m_us_name));
    contact->second.GetAbsolutePointSupposed()->set_active("false");
    contact->second.GetAbsoluteFilteredPointSupposed()->set_label(contact->first + "_filtered_point_supposed_by_" + toupper(m_us_name));
    contact->second.GetAbsoluteFilteredPointSupposed()->set_active("false");
  }
}


//-----------------------------------------------------------
// Procedure: postViewablePoints

void ContactsPing_Formation::postViewablePoints()
{
  int max_filter_size = 0;
  Formation_Behavior::postViewablePoints();
  for(map<string,PingContact>::iterator contact = m_map_contacts.begin() ;
        contact != m_map_contacts.end() ;
        ++contact)
  {
    if(m_display_contact_position_supposed)
    {
      contact->second.GetAbsolutePointSupposed()->set_active(true);
      postMessage("VIEW_POINT", contact->second.GetAbsolutePointSupposed()->get_spec());
    }

    if(m_display_filtered_contact_position_supposed)
    {
      contact->second.GetAbsoluteFilteredPointSupposed()->set_active(true);
      postMessage("VIEW_POINT", contact->second.GetAbsoluteFilteredPointSupposed()->get_spec());
    }

    int filter_size = contact->second.GetCurrentFilterSize();
    if(max_filter_size < filter_size)
      max_filter_size = filter_size;
  }

  postIntMessage("MAX_FILTER_SIZE", max_filter_size);
}


//-----------------------------------------------------------
// Procedure: postErasablePoints

void ContactsPing_Formation::postErasablePoints()
{
  Formation_Behavior::postErasablePoints();
  for(map<string,PingContact>::iterator contact = m_map_contacts.begin() ;
        contact != m_map_contacts.end() ;
        ++contact)
  {
    if(m_display_contact_position_supposed)
    {
      contact->second.GetAbsolutePointSupposed()->set_active(false);
      postMessage("VIEW_POINT", contact->second.GetAbsolutePointSupposed()->get_spec());
    }

    if(m_display_filtered_contact_position_supposed)
    {
      contact->second.GetAbsoluteFilteredPointSupposed()->set_active(false);
      postMessage("VIEW_POINT", contact->second.GetAbsoluteFilteredPointSupposed()->get_spec());
    }
  }
}


//-----------------------------------------------------------
// Procedure: addContactToWhiteList

bool ContactsPing_Formation::addContactToWhiteList(string param_val)
{
  string contact_name = tolower(param_val);
  if(contact_name != "")
  {
    m_map_contacts[contact_name];
    m_map_contacts[contact_name].SetName(contact_name);
    return true;
  }

  else
    return false;
}


//-----------------------------------------------------------
// Procedure: getContact

PingContact* ContactsPing_Formation::getContact(string contact_name)
{
  if(m_map_contacts.count(contact_name) != 0)
    return &(m_map_contacts[contact_name]);

  else
    return NULL;
}
