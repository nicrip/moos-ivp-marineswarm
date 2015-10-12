/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: SphericalCoordinateSystem.h                     */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#ifndef SphericalCoordinateSystem_HEADER
#define SphericalCoordinateSystem_HEADER

class SphericalCoordinateSystem
{
  public:
  
    static bool Cartesian2Spherical(double x, double y, double z, double& rho, double& theta, double& phi);
    static bool Spherical2Cartesian(double rho, double theta, double phi, double& x, double& y, double& z);
};

#endif 
