/* ************************************************************ */
/*   NAME: Nathan Vandervaeren - Simon Rohou                    */
/*   ORGN: MOOSSafir - CGG (Massy - France)                     */
/*   FILE: SolitonCurrent.cpp                                   */
/*   DATE: april 2014                                           */
/* ************************************************************ */

#include <iostream>
#include <cmath>
#include <stdlib.h> // rand()
#include "AngleUtils.h"
#include "SolitonCurrent.h"

using namespace std;


//---------------------------------------------------------
// Procedure: Constructor

SolitonCurrent::SolitonCurrent()
{
  m_wavelength = rand() % 900 + 100;   // 100-1000m
  m_wave_period = rand() % 240 + 480;  // 10min -+ 2min
  m_number_of_waves = rand() % 7 + 4;  // 4-10 waves
  m_first_front_at_zero = 0.;        // First waves coming at MOOSTime() = 0 sec
  m_max_horizontal_speed = 0.8;        // meters/sec
  m_max_vertical_amplitude = 30.0;     // meters
  m_tide_direction = 0;
}


//---------------------------------------------------------
// Procedure: Destructor

SolitonCurrent::~SolitonCurrent()
{

}


//---------------------------------------------------------
// Procedure: setParam

void SolitonCurrent::setParam(string param, string value)
{
  CurrentModel::setParam(param, value);
  
  if(param == "SOLITON__MAX_HORIZONTAL_SPEED")
    setMaxHorizontalSpeed(atof(value.c_str()));
    
  if(param == "SOLITON__MAX_VERTICAL_AMPLITUDE")
    setMaxVerticalAmplitude(atof(value.c_str()));
    
  if(param == "SOLITON__NUMBER_OF_WAVES")
    setNumberOfWaves(atoi(value.c_str()));
    
  if(param == "SOLITON__FIRST_FRONT_AT_ZERO")
    setFirstFrontAtZero(atof(value.c_str()));
    
  if(param == "SOLITON__TIDE_DIRECTION")
    setTideDirection(atof(value.c_str()));
}


//---------------------------------------------------------
// Procedure: init

bool SolitonCurrent::init()
{
  return true;
}

//---------------------------------------------------------
// Procedure: setWavelength

void SolitonCurrent::setWavelength(double value)
{
  m_wavelength = value;
}

//---------------------------------------------------------
// Procedure: setMaxHorizontalSpeed

void SolitonCurrent::setMaxHorizontalSpeed(double value)
{
  m_max_horizontal_speed = value;
}

//---------------------------------------------------------
// Procedure: setMaxVerticalAmplitude

void SolitonCurrent::setMaxVerticalAmplitude(double value)
{
  m_max_vertical_amplitude = value;
}

//---------------------------------------------------------
// Procedure: setWavePeriod

void SolitonCurrent::setWavePeriod(double value)
{
  m_wave_period = value;
}

//---------------------------------------------------------
// Procedure: setNumberofWaves

void SolitonCurrent::setNumberOfWaves(int value)
{
  m_number_of_waves = value;
}

//---------------------------------------------------------
// Procedure: setFirstFrontAtZero

void SolitonCurrent::setFirstFrontAtZero(double time)
{
  m_first_front_at_zero = time;
}

//---------------------------------------------------------
// Procedure: setTideDirection

void SolitonCurrent::setTideDirection(double value)
{
  m_tide_direction = degToRadians(value); // set in degrees, m_tide_direction in radiands
}

//---------------------------------------------------------
// Procedure: type

std::string SolitonCurrent::getType()
{
  return "SOLITON";
}


//---------------------------------------------------------
// Procedure: getCurrent

bool SolitonCurrent::getCurrent(double x, double y, double depth,
                                  double time,
                                  double& drift_x,
                                  double& drift_y,
                                  double& drift_z)
{
  // rotate coordinates x,y,z around z-axis to X,Y,Z so that the waves are propagating in X-direction
  double X =  cos(m_tide_direction) * x + sin(m_tide_direction) * y;
  double Y = -sin(m_tide_direction) * x + cos(m_tide_direction) * y;
  // Z = z
  
  if(parametersOutOfDate(time))
    setNewParameters();
  
  double v = m_wavelength / m_wave_period;    //phase velocity
  double k = 2 * M_PI / m_wavelength;         //wave number
  double alpha = log (1/0.9) / m_wavelength;  //ensure 10% for every wave
  
  double x_in_packet = X - v * (time - m_first_front_at_zero);
  if((x_in_packet < m_wavelength / 4) && (x_in_packet > -(m_number_of_waves -1) * m_wavelength - (m_wavelength * 3 / 4) ))
  {
    double drift_X = m_max_horizontal_speed * cos (k * x_in_packet) * exp (alpha * x_in_packet);
    double drift_Z = m_max_vertical_amplitude * k * v * sin (k * x_in_packet);
    
    // project drift components to original coordinates x,y
    drift_x = cos(m_tide_direction) * drift_X;
    drift_y = sin(m_tide_direction) * drift_X;
    drift_z = drift_Z;
  }
  
  else
  {
    drift_x = 0.0;
    drift_y = 0.0;
    drift_z = 0.0;
  }
  
  return true;
}

//---------------------------------------------------------
// Procedure: parametersOutOfDate
//
// returns true if parameters have not been updated after 6.25 hours since the last packet of waves at (0,0)

bool SolitonCurrent::parametersOutOfDate (double time)
{
  return (time > m_first_front_at_zero + 12.5*3600/2); // if true we change parameters between two packets of internal waves
}

//---------------------------------------------------------
// Procedure: setNewParameters()
//
// chooses a new set of parameters for the next wave packet.
// moves m_first_front_at_zero 12,5 hours later.

void SolitonCurrent::setNewParameters()
{
  m_wavelength = rand() % 900 + 100;   // 100-1000m
  m_wave_period = rand() % 240 + 480;       // 10min -+ 2min
  m_number_of_waves = rand() % 7 + 4;  // 4-10 waves
  m_first_front_at_zero = m_first_front_at_zero + 12.5*3600; // previous + 12.5 hours
}
