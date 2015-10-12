#ifndef SIM_ENGINE_HEADER
#define SIM_ENGINE_HEADER

#include "Node/NodeRecord6DOF.h"
#include "Dynamics.h"

class SimEngine
{
public:
  SimEngine();
  ~SimEngine();
  
public:
  void propagate(NodeRecord6DOF& record,
                 double thrust_force,
                 double rudder_angle,
                 double elevator_angle,
                 double delta_time,
                 double drift_x,
                 double drift_y,
                 double drift_z,
                 double speed_threshold);
                 
protected:
  void NEDtoENU(double& x, double& y, double& z, double& phi, double& theta, double& psi);
  void ENUtoNED(double& x, double& y, double& z, double& phi, double& theta, double& psi);
  
  Dynamics  m_dynamics;
};
#endif


