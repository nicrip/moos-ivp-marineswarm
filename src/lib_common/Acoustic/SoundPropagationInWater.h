/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: SoundPropagationInWater.h                       */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#ifndef SoundPropagationInWater_HEADER
#define SoundPropagationInWater_HEADER

class SoundPropagationInWater
{
  public:
  
    static double GetTheoreticalSpeed(double depth, double temperature, double salinity);
};

#endif 
