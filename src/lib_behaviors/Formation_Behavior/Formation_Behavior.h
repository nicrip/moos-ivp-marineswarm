/* ******************************************************** */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: Formation_Behavior.h                            */
/*    DATE: April 2014                                      */
/*    AUTH: Simon Rohou                                     */
/* ******************************************************** */
 
#ifndef Formation_Behavior_HEADER
#define Formation_Behavior_HEADER

  #include <list>
  #include <cmath>
  #include <string>
  #include <cstdlib>
  #include <iostream>
  #include "AngleUtils.h"
  #include "GeomUtils.h"
  #include "IvPBehavior.h"
  #include "IvPDomain.h"
  #include "XYCircle.h"
  #include "XYPoint.h"
  #include "MOOS/libMOOS/MOOSLib.h"

  class IvPDomain;
  class Formation_Behavior : public IvPBehavior
  {
    public:
      Formation_Behavior(IvPDomain);
      ~Formation_Behavior() {};
      
      IvPFunction*  onRunState();
      bool          setParam(std::string, std::string);
      void          onSetParamComplete();
      void          onRunToIdleState();

    protected:
      bool          updatePlatformInfo();
      void          updateAbsoluteTargetPoint(bool active);
      IvPFunction*  IvPFunctionWhenInsideIntRadius(double heading);
      IvPFunction*  IvPFunctionWhenInsideExtRadius(double heading, double dist_from_targetpoint);
      IvPFunction*  IvPFunctionWhenOutsideExtRadius(double heading);
      void          postViewablePoints();
      void          postErasablePoints();
      void          updateStatistics();
      void          filterCentroidTargetpoint();
      
      static void   increaseActiveRepositioningBehaviors();
      static void   decreaseActiveRepositioningBehaviors();
      double        getTotalTimeRepositioning();
      double        getTotalTimeDrifting();
      
      // Ownship
      double        m_heading;
      double        m_nav_depth;
      double        m_nav_x, m_nav_y; // only used for display

      // Formation
      double        m_max_speed;
      double        m_int_radius;
      double        m_ext_radius;
      XYPoint       m_relative_target_point;
      XYPoint       m_absolute_target_point;
      bool          m_drift_passively;
      int           m_filter_size_targetpoints;
      std::list<XYPoint> m_list_previous_targetpoints;
      
      // Display
      bool          m_display_targetpoints;
      bool          m_display_radii;
      bool          m_display_statistics;
      bool          m_decaying_speed_ratio;
      XYPoint       m_label_statistics;
      XYCircle      m_view_int_radius;
      XYCircle      m_view_ext_radius;
      
      // Statistics
      static int    nb_formation_behaviors_;
      static int    nb_active_repositioning_behaviors_;
      static double total_time_repositioning_;
      static double total_time_drifting_;
      static double drifting_date_;
      static double repositioning_date_;
  };

#endif