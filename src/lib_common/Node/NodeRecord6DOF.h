#ifndef NODE_RECORD_6DOF_HEADER
#define NODE_RECORD_6DOF_HEADER

#include <string>
#include <map>
#include "NodeRecord.h"

class NodeRecord6DOF : public NodeRecord
{
 public:
  NodeRecord6DOF(std::string vname="", std::string vtype="");
  ~NodeRecord6DOF() {};

  void setZ(double v)        {m_z=v;       m_z_set=true;};
  void setPhi(double v)       {m_phi=v;     m_phi_set=true;}; 
  void setTheta(double v)     {m_theta=v;   m_theta_set=true;}; 
  void setPsi(double v)       {m_psi=v;     m_psi_set=true;}; 
  void setSurge(double v)     {m_surge=v;   m_surge_set=true;}; 
  void setSway(double v)      {m_sway=v;    m_sway_set=true;}; 
  void setHeave(double v)     {m_heave=v;   m_heave_set=true;}; 
  void setRoll(double v)      {m_roll=v;    m_roll_set=true;}; 
  void setPitch(double v)     {m_pitch=v;   m_pitch_set=true;}; 
  void setYaw(double v)       {m_yaw=v;     m_yaw_set=true;}; 
  
  double getZ() const          {return(m_z);}; 
  double getPhi() const        {return(m_phi);}; 
  double getTheta() const      {return(m_theta);}; 
  double getPsi() const        {return(m_psi);}; 
  double getSurge() const      {return(m_surge);}; 
  double getSway() const       {return(m_sway);}; 
  double getHeave() const      {return(m_heave);}; 
  double getRoll() const       {return(m_roll);}; 
  double getPitch() const      {return(m_pitch);}; 
  double getYaw() const        {return(m_yaw);}; 
  
  bool isSetZ() const          {return(m_z_set);}; 
  bool isSetPhi() const        {return(m_phi_set);}; 
  bool isSetTheta() const      {return(m_theta_set);}; 
  bool isSetPsi() const        {return(m_psi_set);}; 
  bool isSetSurge() const      {return(m_surge_set);}; 
  bool isSetSway() const       {return(m_sway_set);}; 
  bool isSetHeave() const      {return(m_heave_set);}; 
  bool isSetRoll() const       {return(m_roll_set);}; 
  bool isSetPitch() const      {return(m_pitch_set);}; 
  bool isSetYaw() const        {return(m_yaw_set);}; 

 protected:
  double m_z;
  double m_phi;
  double m_theta; 
  double m_psi;
  double m_surge; 
  double m_sway; 
  double m_heave; 
  double m_roll;
  double m_pitch; 
  double m_yaw; 
  
  bool m_z_set; 
  bool m_phi_set;
  bool m_theta_set; 
  bool m_psi_set;
  bool m_surge_set; 
  bool m_sway_set; 
  bool m_heave_set; 
  bool m_roll_set; 
  bool m_pitch_set; 
  bool m_yaw_set; 
};

#endif 


