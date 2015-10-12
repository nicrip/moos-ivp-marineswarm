/************************************************************/
/*    NAME: Nick Rypkema                                    */
/*    ORGN: MIT                                             */
/*    FILE: BHV_DriftingForces.h                            */
/*    DATE:                                                 */
/************************************************************/

#ifndef DriftingForces_HEADER
#define DriftingForces_HEADER

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

class BHV_DriftingForces : public virtual IvPBehavior {
public:
  BHV_DriftingForces(IvPDomain);
  ~BHV_DriftingForces() {};

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
  IvPFunction*  IvPFunctionInsideCaptureForce(double heading);
  IvPFunction*  IvPFunctionInsideDriftingForce(double heading, double dist_from_target);
  IvPFunction*  IvPFunctionOutsideDriftingForce(double heading);
  void          postViewablePoints();
  void          postErasablePoints();
  void          addRelativeForceSource(double x, double y, double weight);
  void          clearForceSources();
  void          relativeForceSourceToForce(double x, double y);
  double        vanDerWaals(double eps, double A, double B, int alpha, int beta, double dist);

protected: // Configuration parameters
  double        m_speed;
  double        m_max_force;
  double        m_capture_force;
  double        m_slip_force;
  double        m_drifting_force;
  bool          m_display_force_sources;
  bool          m_display_total_force;
  bool          m_display_partial_forces;
  bool          m_display_statistics;
  bool          m_display_drift;

protected: // State variables
  double        m_nav_heading;
  double        m_nav_depth;
  double        m_nav_x;
  double        m_nav_y;
  double        m_energy;
  double        m_drift_x;
  double        m_drift_y;
  bool          m_drifting;
  XYPoint       m_statistics_label;
  XYVector      m_drift_vector;
  double        m_rel_summed_force_x;
  double        m_rel_summed_force_y;
  XYVector      m_rel_summed_force_vector;
  double        m_rel_force_x;
  double        m_rel_force_y;
  XYVector       m_rel_force;
  std::list<XYVector> m_rel_forces;
  XYPoint       m_force_source;
  std::list<XYPoint> m_force_sources;
};

#endif
