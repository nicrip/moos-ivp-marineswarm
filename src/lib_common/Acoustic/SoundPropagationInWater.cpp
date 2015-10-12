/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: SoundPropagationInWater.cpp                     */
/*    DATE: february 2014                                   */
/* ******************************************************** */

#include <math.h>
#include "SoundPropagationInWater.h"

//---------------------------------------------------------
// Procedure: GetTheoreticalSoundSpeedInWater
// The sound speed in water is calculate with:
//    - temperature (°C), depth (m), salinity (default value in header file)

double SoundPropagationInWater::GetTheoreticalSpeed(double depth, double temperature, double salinity)
{
  // cf TMR4230 Oceanography Current (by Dag Myrhaug, NTNU professor)
  //     1.3. sound propagation in sea water
  return 1448.6 + 
          4.618 * temperature - 0.0523 * pow(temperature, 2) + 
          1.25 * (salinity - 35.) + 
          0.017 * depth;
}
