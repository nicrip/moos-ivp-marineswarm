/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: SimulateNodeLoss.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef SimulateNodeLoss_HEADER
#define SimulateNodeLoss_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class SimulateNodeLoss : public CMOOSApp
{
 public:
   SimulateNodeLoss();
   ~SimulateNodeLoss();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

 private: // Configuration variables
   double       m_delta_time;
   int          m_num_nodes_loss;
   int          m_num_nodes;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   double       m_start_MOOSTime;
   double       m_previous_MOOSTime;
   bool         m_deploy;
   bool         m_start_loss;
   int          m_num_loss;
   std::vector<int> m_lost_nodes;
};

#endif
