/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForce.h                             */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingForce_HEADER
#define DriftingForce_HEADER

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

class BHV_DriftingForce : public virtual IvPBehavior {
public:
  BHV_DriftingForce(IvPDomain);
  ~BHV_DriftingForce() {};

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
  IvPFunction*  IvPFunctionInsideCaptureForce(double heading, double speed);
  IvPFunction*  IvPFunctionInsideDriftingForce(double heading, double speed);
  IvPFunction*  IvPFunctionOutsideDriftingForce(double heading, double speed);
  void          postViewablePoints();
  void          postErasablePoints();
  void          setForce(double force_x, double force_y);
  static void   increaseDriftingTargetsCauseThrusting();
  static void   decreaseDriftingTargetsCauseThrusting();
  double        getTimeThrusting();
  double        getTimeDrifting();
  void          updateStatistics();

protected: // Configuration parameters
  double        m_speed;
  double        m_max_force;
  double        m_capture_force;
  double        m_slip_force;
  double        m_drifting_force;
  bool          m_display_force;
  bool          m_display_statistics;
  bool          m_display_drift;

protected: // State variables
  double        m_nav_heading;
  double        m_nav_depth;
  double        m_nav_x;
  double        m_nav_y;
  double        m_capture_speed;
  double        m_slip_speed;
  double        m_drifting_speed;
  double        m_energy;
  double        m_drift_x;
  double        m_drift_y;
  bool          m_drifting;
  XYPoint       m_statistics_label;
  XYVector      m_drift_vector;
  double        m_force_x;
  double        m_force_y;
  XYVector      m_force_vector;
  XYVector      m_capture_force_vector;
  XYVector      m_slip_force_vector;
  XYVector      m_drifting_force_vector;
  double        m_force_speed;
  double        m_force_heading;
  double        m_force_magnitude;

protected:  // Static variables (shared across all inherited classes)
  static int    _num_drifting_targets;
  static int    _num_drifting_targets_cause_thrusting;
  static double _time_thrusting_startclock;
  static double _time_thrusting;
  static double _time_drifting_startclock;
  static double _time_drifting;
};

#endif
