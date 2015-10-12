/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ConvexHullTest.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef ConvexHullTest_HEADER
#define ConvexHullTest_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "Geometry/IncrementalConvexHull.h"
#include "XYPolygon.h"

class ConvexHullTest : public CMOOSApp
{
 public:
   ConvexHullTest();
   ~ConvexHullTest();

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
   XYPolygon    m_abs_targets;
   double       m_new_x;
   double       m_new_y;
   bool         m_new_pt;
   bool         m_reset;
   IncrementalConvexHull m_ch;
};

#endif
