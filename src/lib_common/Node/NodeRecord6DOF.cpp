
#include "NodeRecord6DOF.h"
#include "MBUtils.h"

using namespace std;

//---------------------------------------------------------
// Constructor

NodeRecord6DOF::NodeRecord6DOF(string vname, string vtype) : NodeRecord(vname, vtype)
{
  m_z          = 0; 
  m_phi        = 0;
  m_theta      = 0; 
  m_psi        = 0;
  m_surge      = 0; 
  m_sway       = 0; 
  m_heave      = 0; 
  m_roll       = 0;
  m_pitch      = 0;
  m_yaw        = 0;
  
  m_z_set       = false; 
  m_phi_set     = false;
  m_theta_set   = false; 
  m_psi_set     = false; 
  m_surge_set   = false; 
  m_sway_set    = false; 
  m_heave_set   = false; 
  m_roll_set    = false; 
  m_pitch_set   = false; 
  m_yaw_set     = false; 
}

