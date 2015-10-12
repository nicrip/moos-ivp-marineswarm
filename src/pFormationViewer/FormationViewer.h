/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: FormationViewer.h                               */
/*    DATE:                                                 */
/************************************************************/

#ifndef FormationViewer_HEADER
#define FormationViewer_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <map>
#include "NodeRecord.h"

class FormationViewer : public AppCastingMOOSApp
{
 public:
   FormationViewer();
   ~FormationViewer();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();
   bool HandleNewNodeReport(CMOOSMsg msg);
   bool buildReport();

 private: // Configuration variables
   std::vector< std::pair<std::string,std::string> > m_vector_node_pair;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   std::map< std::string, NodeRecord > m_map_node_records;    // <vehicle_name, node_record>
   int          m_num_communities;
};

#endif
