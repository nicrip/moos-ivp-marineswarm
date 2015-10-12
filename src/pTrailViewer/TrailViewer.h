/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: TrailViewer.h                                   */
/*    DATE:                                                 */
/************************************************************/

#ifndef TrailViewer_HEADER
#define TrailViewer_HEADER

#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "XYPoint.h"
#include "XYSegList.h"

class TrailViewer : public CMOOSApp
{
 public:
   TrailViewer();
   ~TrailViewer();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

 private: // Configuration variables
   double       m_distance;

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
   XYPoint      m_pos;
   XYPoint      m_prev_pos;
   XYSegList    m_trail;
   std::string  m_name;
   bool         m_init_x;
   bool         m_init_y;
   bool         m_init_name;
   ColorPack    m_color;
};

#endif
