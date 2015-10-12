/* ******************************************************** */
/*   NAME: Simon Rohou                                      */
/*   ORGN: MOOSSafir - CGG (Massy - France)                 */
/*   FILE: ContactsViewer_CGG.h                             */
/*   DATE: June 2014                                        */
/* ******************************************************** */

#ifndef ContactsViewer_HEADER
#define ContactsViewer_HEADER

#include <map>
#include <vector>
#include <string>
#include <utility>
#include "NodeRecord.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class ContactsViewer : public AppCastingMOOSApp
{
  public:
    ContactsViewer();
    ~ContactsViewer();

  protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void RegisterVariables();
    bool buildReport();
    bool HandleNewNodeReport(CMOOSMsg msg);
    void DisplaySegment(std::pair<std::string,std::string> couple_of_contact);
    
  private:
    std::map<std::string,int> m_map_contacts;
    std::map<std::string,bool> m_map_contacts_deployed;
    std::vector<std::pair<std::string,std::string> > m_vector_couple_of_contact;
    std::map<std::string,NodeRecord> m_map_node_records;
    std::map<std::string,int> m_map_segments;
    int m_updates_display, m_updates_report;
};

#endif 
