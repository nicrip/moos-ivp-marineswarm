#ifndef USM_MODEL_HEADER
#define USM_MODEL_HEADER

#include <string>
#include "Node/NodeRecord6DOF.h"
#include "MBTimer.h"
#include "SimEngine.h"

class USM_Model
{
public:
  USM_Model();                                          
  virtual  ~USM_Model() {};                             

  bool   propagate(double time);
  void   resetTime(double time);                             
  
  // Setters
  bool   setParam(std::string, double);

  void   setPaused(bool);   
  
  void   setThrustForce(double v)          {m_thrust_force = v;};
  void   setRudderAngle(double v)          {m_rudder_angle = v;};
  void   setElevatorAngle(double v)        {m_elevator_angle = v;};
    
  void   setMaxThrustForce(double v)          {m_max_thrust_force = v;};
  void   setMaxRudderAngle(double v)          {m_max_rudder_angle = v;};
  void   setMaxElevatorAngle(double v)        {m_max_elevator_angle = v;};
  
  void   setDriftX(double v)        {m_drift_x = v;};
  void   setDriftY(double v)        {m_drift_y = v;};
  void   setDriftZ(double v)        {m_drift_z = v;};

  bool   initPosition(const std::string&);

  // Getters
  double     getThrustForce() const   {return(m_thrust_force);};
  double     getRudderAngle() const   {return(m_rudder_angle);};
  double     getElevatorAngle() const {return(m_elevator_angle);};
  
  double     getMaxThrustForce() const   {return(m_max_thrust_force);};
  double     getMaxRudderAngle() const   {return(m_max_rudder_angle);};
  double     getMaxElevatorAngle() const {return(m_max_elevator_angle);};
  
  double     getDriftX() const       {return(m_drift_x);};
  double     getDriftY() const       {return(m_drift_y);};
  double     getDriftZ() const       {return(m_drift_z);};
  
  double     getDriftMag() const;
  double     getDriftAng() const;
  
  double     getWaterDepth() const   {return(m_water_depth);};
  
  NodeRecord6DOF getNodeRecord() const   {return(m_record);};
  
  std::string getDriftSummary();

 protected:
  void   propagateNodeRecord6DOF(NodeRecord6DOF& record, double delta_time);

 protected:

  bool       m_paused;          
  
  double     m_thrust_force;      // newtons
  double     m_rudder_angle;      // rad
  double     m_elevator_angle;    // rad
  
  double     m_max_thrust_force;  // newtons
  double     m_max_rudder_angle;  // rad
  double     m_max_elevator_angle;// rad

  double     m_water_depth;

  double     m_drift_x;           // meters per sec
  double     m_drift_y;           // meters per sec
  double     m_drift_z;           // meters per sec
  
  double     m_speed_threshold;

  NodeRecord6DOF m_record;       // NAV_X, NAV_Y, etc 

  MBTimer    m_pause_timer;
  SimEngine  m_simengine;
};
#endif


