/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: ContactsViewer_CGG.cpp                           */
/*   DATE: June 2014                                        */
/* ******************************************************** */

#include <iterator>
#include <fstream>
#include "MBUtils.h"
#include "ContactsViewer_CGG.h"
#include "NodeRecordUtils.h"
#include "XYSegList.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ContactsViewer::ContactsViewer()
{
  m_updates_display = 0;
  m_updates_report = 0;
}


//---------------------------------------------------------
// Destructor

ContactsViewer::~ContactsViewer()
{
  
}


//---------------------------------------------------------
// Procedure: OnNewMail

bool ContactsViewer::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;

  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    
    if(msg.GetKey() == "NODE_REPORT")
    {
      m_map_contacts[msg.GetCommunity()] ++;
      
      if(m_map_contacts_deployed.count(msg.GetCommunity()) == 0)
        m_map_contacts_deployed[msg.GetCommunity()] = false;
      
      return HandleNewNodeReport(msg);
    }
    
    #if 0 // Keep these around just for template
      string key   = msg.GetKey();
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString(); 
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif
  }

  return true;
}


//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ContactsViewer::OnConnectToServer()
{
  RegisterVariables();
  return true;
}


//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ContactsViewer::Iterate()
{
  AppCastingMOOSApp::Iterate();  
  AppCastingMOOSApp::PostReport();
    
  return true;
}


//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ContactsViewer::OnStartUp()
{
  int current_launch_point = -1;
  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
  {
    sParams.reverse();
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++)
    {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
      
      if(param == "DISPLAY_CONTACTS_SEGMENT")
      {
        string sub_values[2] = { "", "" };
        
        int k = 0;
        for(int i = 0 ; i < value.length() ; i++)
        {
          if(value[i] == ',')
          {
            k = 1;
            continue;
          }
          
          sub_values[k] += value[i];
        }
    
        m_vector_couple_of_contact.push_back(make_pair(tolower(sub_values[0]), tolower(sub_values[1])));
      }
    }
  }
  
  RegisterVariables(); 
  return true;
}


//---------------------------------------------------------
// Procedure: RegisterVariables

void ContactsViewer::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NODE_REPORT", 0.0);
}


//---------------------------------------------------------
// Procedure: buildReport

bool ContactsViewer::buildReport()
{
  m_msgs << endl;
  m_msgs << "  pContactsViewer is running (updates - display: " << m_updates_display << " report: " << m_updates_report << ")" << endl;
  
  m_msgs << endl << "  Config:";
  
  for(vector<pair<string,string> >::iterator couple = m_vector_couple_of_contact.begin() ; 
      couple != m_vector_couple_of_contact.end() ; 
        couple++)
  {
    m_msgs << endl << "    " << (*couple).first << " - " << (*couple).second;
  }
  
  m_msgs << endl << endl << "  Contacts:";
  
  for(map<string,int>::iterator seg = m_map_contacts.begin() ; 
      seg != m_map_contacts.end() ; 
        seg++)
  {
    m_msgs << endl << "    " << (*seg).first << " (" << (*seg).second << ")";
  }
  
  m_msgs << endl << endl << "  Segments:";
  
  for(map<string,int>::iterator seg = m_map_segments.begin() ; 
      seg != m_map_segments.end() ; 
        seg++)
  {
    m_msgs << endl << "    " << (*seg).first << " (" << (*seg).second << ")";
  }
  
  return true;
}


//---------------------------------------------------------
// Procedure: DisplaySegment

void ContactsViewer::DisplaySegment(pair<string,string> couple_of_contact)
{
  XYSegList seg_list;
  seg_list.add_vertex(m_map_node_records[couple_of_contact.first].getX(),
                      m_map_node_records[couple_of_contact.first].getY());
  
  seg_list.add_vertex(m_map_node_records[couple_of_contact.second].getX(),
                      m_map_node_records[couple_of_contact.second].getY());
                      
  string label = couple_of_contact.first + couple_of_contact.second;
  
  seg_list.set_label(label);
  seg_list.set_active(true);
  seg_list.set_vertex_size(4);
  seg_list.set_time(MOOSTime());
  seg_list.set_edge_size(1);
  seg_list.set_edge_color("cyan");
  //seg_list.set_transparency(0.);
  
  m_map_segments[label] ++;
  
  Notify("VIEW_SEGLIST", seg_list.get_spec());
}


//---------------------------------------------------------
// Procedure: HandleNewNodeReport

bool ContactsViewer::HandleNewNodeReport(CMOOSMsg msg)
{
  string message_value = msg.GetString();
  string community_name = msg.GetCommunity();
  
  for(vector<pair<string,string> >::iterator couple = m_vector_couple_of_contact.begin() ; 
      couple != m_vector_couple_of_contact.end() ; 
        couple++)
  {
    if(community_name == (*couple).first || community_name == (*couple).second)
    {
      m_map_node_records[community_name] = string2NodeRecord(message_value);
      
      if(!m_map_contacts_deployed[community_name])
      {
        if(m_map_node_records[community_name].getSpeed() != 0.)
          m_map_contacts_deployed[community_name] = true;
      }
      
      m_updates_report ++;
      break;
    }
  }
  
  for(vector<pair<string,string> >::iterator couple = m_vector_couple_of_contact.begin() ; 
      couple != m_vector_couple_of_contact.end() ; 
        couple++)
  {
    if(community_name == (*couple).first || community_name == (*couple).second)
    {
      // we only display segments between deployed nodes
      if(m_map_contacts_deployed[(*couple).first] && m_map_contacts_deployed[(*couple).second])
      {
        DisplaySegment((*couple));
        m_updates_display ++;
      }
    }
  }
  
  return true;
}
