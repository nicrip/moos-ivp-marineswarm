/* ******************************************************** */
/* ORGN: MOOSSafir - CGG (Massy - France)                   */
/* FILE: VirtualStructure_MapPingFormation.cpp              */
/* DATE: July 2014                                          */
/* AUTH: Nathan Vandervaeren                                */
/* ******************************************************** */

#include <armadillo>
#include "XYCommsPulse.h"
#include "MBUtils.h"
#include "VirtualStructure_MapPingFormation.h"

using namespace std;
using namespace arma;


//-----------------------------------------------------------
// Procedure: Constructor
VirtualStructure_MapPingFormation::VirtualStructure_MapPingFormation(IvPDomain gdomain) : Map_PingFormation(gdomain)
{
  setParam("name", "VirtualStructure_MapPingFormation");
  addInfoVars("PING_REPORT");
}


//-----------------------------------------------------------
// Procedure: setParam

bool VirtualStructure_MapPingFormation::setParam(string param, string param_val)
{
  if(Map_PingFormation::setParam(param, param_val))
    return true;
    
#if 0 // keep it here as a template
  double dval = atof(param_val.c_str());
  bool non_neg_number = (isNumber(param_val) && (dval >= 0));

  if(param == "additional_parameter" && isNumber(param_val))
  {
    m_additional_parameter = dval;
    return true;
  }
#endif

  return false;
}


//-----------------------------------------------------------
// Procedure: onRunState

IvPFunction *VirtualStructure_MapPingFormation::onRunState()
{
  if(!Map_PingFormation::updatePlatformInfo())
    return 0;

  // Centroid of real positions 
  double xCentroid_r = 0.0;
  double yCentroid_r = 0.0;

  // Centroid of desired positions
  double xCentroid_d = 0.0;
  double yCentroid_d = 0.0;

  // Go through all the valid contacts build centroids
  // We add the vehicle itself (0,0) to the centroids
  int number_of_valid_contacts = 1;
  
  for(map<string,PingContact>::iterator ping_contact = m_map_contacts.begin() ;
      ping_contact != m_map_contacts.end() ;
      ping_contact++)
  {
  	if(ping_contact->second.HasExpired())
    continue;
    
    number_of_valid_contacts++;

    // Desired position of the contact
    double x_d = m_map_swarm_elements_position[ping_contact->first].x();
    double y_d = m_map_swarm_elements_position[ping_contact->first].y();

    // Real position of the contact
    double x_r = ping_contact->second.GetRelativeX();
    double y_r = ping_contact->second.GetRelativeY();
    
    // Add the contact to centroids
    xCentroid_d += x_d;
    yCentroid_d += y_d;
    xCentroid_r += x_r;
    yCentroid_r += y_r;
  }
  
  // Divide by the number of valid contacts
  xCentroid_d *= 1. / number_of_valid_contacts;
  yCentroid_d *= 1. / number_of_valid_contacts;
  xCentroid_r *= 1. / number_of_valid_contacts;
  yCentroid_r *= 1. / number_of_valid_contacts;
  
  // Centroids as matrices (for later)
  mat matrixCentroid_d = mat(2,1);
  mat matrixCentroid_r = mat(2,1);
  
  matrixCentroid_d(0,0) = xCentroid_d;
  matrixCentroid_d(1,0) = yCentroid_d;
  matrixCentroid_r(0,0) = xCentroid_r;
  matrixCentroid_r(1,0) = yCentroid_r;
  
  // Covariance matrix_H
  mat matrix_H  = mat(2,2); // covariance matrix
  matrix_H.zeros();         // initialized with zeros

  // Going through all the valid contacts again to build covariance matrix H
  for(map<string,PingContact>::iterator ping_contact = m_map_contacts.begin() ;
      ping_contact != m_map_contacts.end() ;
      ping_contact++)
  {
  	if(ping_contact->second.HasExpired())
    continue;
  	
  	// Desired position of the contact
    double x_d = m_map_swarm_elements_position[ping_contact->first].x();
    double y_d = m_map_swarm_elements_position[ping_contact->first].y();

    // Real position of the contact
    double x_r = ping_contact->second.GetRelativeX();
    double y_r = ping_contact->second.GetRelativeY();
    
    // Positions as matrices (for later)
    mat matrix_d  = mat(2,1);
    mat matrix_r  = mat(2,1);

    matrix_d(0,0) = x_d;
    matrix_d(1,0) = y_d;
		matrix_r(0,0) = x_r;
		matrix_r(1,0) = y_r;

    // Increase covariance matrix
		matrix_H = matrix_H + (matrix_d - matrixCentroid_d) * trans(matrix_r - matrixCentroid_r);
	}

  // Find matrix_R and matrix_t from the Singular Value Decomposition of matrix_H

  mat U;
  vec s;
  mat V;
  svd(U, s, V, matrix_H); // Singular Value Decomposition
  mat matrix_R = V * trans(U);

  // Check if det(matrix_R) is > 0
  // If so, multiply the last column by -1

  if (det(matrix_R) < 0)
  {
    mat I = mat(2,2);
    I.zeros();
    I(0,0) = 1.0;
    I(1,1) = -1.0;
    matrix_R = matrix_R * I; // multiply the 2nd column by -1
  }
  
  mat matrix_t = -(matrix_R * matrixCentroid_d) + matrixCentroid_r;

  // Calculate target point
  mat target_point = matrix_t;
  double target_x = target_point(0,0);
  double target_y = target_point(1,0);

  ostringstream strs;
  strs << target_x << "," << target_y;

  postMessage("TOTAL_NEIGHBORS", number_of_valid_contacts);
  postMessage("RELATIVE_TARGET_POINT", strs.str());

  m_relative_target_point.set_vertex(target_x, target_y);
  filterCentroidTargetpoint();
  
  return Map_PingFormation::onRunState();
}
