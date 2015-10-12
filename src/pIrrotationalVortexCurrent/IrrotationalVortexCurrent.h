/************************************************************/
/*    NAME: Nick Rypkema                                              */
/*    ORGN: MIT                                             */
/*    FILE: IrrotationalVortexCurrent.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef IrrotationalVortexCurrent_HEADER
#define IrrotationalVortexCurrent_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class IrrotationalVortexCurrent : public CMOOSApp
{
 public:
   IrrotationalVortexCurrent();
   ~IrrotationalVortexCurrent();

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
