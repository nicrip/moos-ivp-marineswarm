/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: SphericalCoordinateSystem.cpp                   */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#include <math.h>
#include <iostream>

#include "SphericalCoordinateSystem.h"

bool SphericalCoordinateSystem::Cartesian2Spherical(double x, double y, double z, double& rho, double& theta, double& phi)
{
  rho = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
  
  if(rho != 0)
  {
    theta = acos(z / rho);
    phi = acos(x / sqrt(pow(x, 2) + pow(y, 2)));

    if(y < 0)
      phi = (2. * M_PI) - phi;
  }
  
  else
  {
    theta = 0;
    phi = 0;
  }
  
  return true;
}

bool SphericalCoordinateSystem::Spherical2Cartesian(double rho, double theta, double phi, double& x, double& y, double& z)
{
  if(rho < 0)
  {
    std::cout << "Negative radius (Spherical2Cartesian): rho = " << rho << std::endl;
    return false;
  }
  
  x = rho * sin(theta) * cos(phi);
  y = rho * sin(theta) * sin(phi);
  z = rho * cos(theta);
  
  return true;
}
