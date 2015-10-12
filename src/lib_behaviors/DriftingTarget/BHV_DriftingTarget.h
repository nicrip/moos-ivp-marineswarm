/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*          Modification of Formation_Behaviour by Simon Rohou */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingTarget.h                            */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingTarget_HEADER
#define DriftingTarget_HEADER

#include <string>
#include <list>
#include <cmath>
#include <string>
#include <iostream>
#include "IvPBehavior.h"
#include "AngleUtils.h"
#include "GeomUtils.h"
#include "IvPDomain.h"
#include "XYCircle.h"
#include "XYPoint.h"
#include "XYVector.h"
#include "MOOS/libMOOS/MOOSLib.h"

class BHV_DriftingTarget : public virtual IvPBehavior {
public:
  BHV_DriftingTarget(IvPDomain);
  ~BHV_DriftingTarget() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  bool          updateInfoIn();
  IvPFunction*  IvPFunctionInsideCaptureRadius(double heading);
  //IvPFunction*  IvPFunctionInsideSlipRadius(double heading, double dist_from_target);
  IvPFunction*  IvPFunctionInsideDriftingRadius(double heading, double dist_from_target);
  IvPFunction*  IvPFunctionOutsideDriftingRadius(double heading);
  void          postViewablePoints();
  void          postErasablePoints();
  void          updateViewUnfilteredPoints(bool display);
  void          addRelativeTargetToFilter(double x, double y);
  void          filterCentroidTargets();
  static void   increaseDriftingTargetsCauseThrusting();
  static void   decreaseDriftingTargetsCauseThrusting();
  double        getTimeThrusting();
  double        getTimeDrifting();
  void          updateStatistics();

protected: // Configuration parameters
  double        m_speed;
  double        m_capture_radius;
  double        m_slip_radius;
  double        m_drifting_radius;
  int           m_targets_filter_size;
  bool          m_display_filtered_target;
  bool          m_display_unfiltered_targets;
  bool          m_display_radii;
  bool          m_display_statistics;
  bool          m_display_drift;
  double        m_end_speed_scale;

protected: // State variables
  double        m_nav_heading;
  double        m_nav_depth;
  double        m_nav_x;
  double        m_nav_y;
  double        m_energy;
  double        m_dist_to_target;
  double        m_angle_to_target;
  double        m_drift_x;
  double        m_drift_y;
  XYPoint       m_relative_target;
  XYPoint       m_absolute_target;
  bool          m_drifting;
  std::list<XYPoint> m_targets;
  XYPoint       m_statistics_label;
  XYCircle      m_capture_radius_circle;
  XYCircle      m_slip_radius_circle;
  XYCircle      m_drifting_radius_circle;
  XYVector      m_drift_vector;

protected:  // Static variables (shared across all inherited classes)
  static int    _num_drifting_targets;
  static int    _num_drifting_targets_cause_thrusting;
  static double _time_thrusting_startclock;
  static double _time_thrusting;
  static double _time_drifting_startclock;
  static double _time_drifting;
};

#endif
