/* ******************************************************** */
/*    NAME: Simon Rohou                                     */
/*    ORGN: MOOSSafir - CGG (Massy - France)                */
/*    FILE: CoupleOfContact.cpp                             */
/*    DATE: april 2014                                      */
/* ******************************************************** */

#include <cmath>
#include "AngleUtils.h"
#include "GeomUtils.h"
#include "CoupleOfContact.h"
#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;    

//-----------------------------------------------------------
// Procedure: Constructor

CoupleOfContact::CoupleOfContact(std::string name)
{
  setName(name);
}


//-----------------------------------------------------------
// Procedure: setName

void CoupleOfContact::setName(std::string name)
{
  m_name = name;
}


//-----------------------------------------------------------
// Procedure: getName

std::string CoupleOfContact::getName()
{
  return m_name;
}


//-----------------------------------------------------------
// Procedure: getContact1

PingContact* CoupleOfContact::getContact1()
{
  return m_contact_1;
}


//-----------------------------------------------------------
// Procedure: getContact2

PingContact* CoupleOfContact::getContact2()
{
  return m_contact_2;
}


//-----------------------------------------------------------
// Procedure: setContact1

void CoupleOfContact::setContact1(PingContact* contact)
{
  m_contact_1 = contact;
}


//-----------------------------------------------------------
// Procedure: setContact1

void CoupleOfContact::setContact2(PingContact* contact)
{
  m_contact_2 = contact;
}


//-----------------------------------------------------------
// Procedure: setDistFromMidpoint

void CoupleOfContact::setDistFromMidpoint(double dist_from_midpoint)
{
  m_dist_from_midpoint = dist_from_midpoint;
}


//-----------------------------------------------------------
// Procedure: setWeight

void CoupleOfContact::setWeight(double weight)
{
  m_weight = weight;
}


//-----------------------------------------------------------
// Procedure: getTargetPoint

XYPoint CoupleOfContact::getTargetPoint()
{
  double xA = m_contact_1->GetRelativeX();
  double yA = m_contact_1->GetRelativeY();
  double xB = m_contact_2->GetRelativeX();
  double yB = m_contact_2->GetRelativeY();
  
  double xC, yC, angle;
  
  xC = (xB + xA) / 2.;
  yC = (yB + yA) / 2.;
  angle = relAng(xA, yA, xB, yB);
  
  // Calculate the target point
  double targetpoint_position_x, targetpoint_position_y;
  
  projectPoint(angle360(angle + 90.), m_dist_from_midpoint, 
                xC, yC,
                targetpoint_position_x, targetpoint_position_y);
  
  return XYPoint(targetpoint_position_x, targetpoint_position_y);
}


//-----------------------------------------------------------
// Procedure: getWeight

double CoupleOfContact::getWeight()
{
  return m_weight;
}


//-----------------------------------------------------------
// Procedure: IsInitialized

bool CoupleOfContact::IsInitialized()
{
  return m_contact_1->IsInitialized() && m_contact_2->IsInitialized();
}
