/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: ThreeChannelCurrent.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef ThreeChannelCurrent_HEADER
#define ThreeChannelCurrent_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class ThreeChannelCurrent : public CMOOSApp
{
 public:
   ThreeChannelCurrent();
   ~ThreeChannelCurrent();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

 private: // Configuration variables

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   double       m_nav_x;
   double       m_nav_y;
};

#endif
