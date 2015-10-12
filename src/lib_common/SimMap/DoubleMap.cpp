/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ThrustMap.cpp                                        */
/*    DATE: Dec 16th 2010                                        */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

/* Modified by Nathan to create a generic tool for mapping       */
/* all the procedure are preserved but renamed                   */
/* new procedure setPoints is added                              */
/* The mapping does not have to be ascending anymore             */
/* However isAscending is kept (can be useful)                   */

#include <cstdlib>
#include <iostream>
#include "DoubleMap.h"
#include "MBUtils.h"

using namespace std;

//----------------------------------------------------------------
// Constructor

DoubleMap::DoubleMap()
{
  m_reflect_negative = false;

  m_figlog.setLabel("DoubleMap");

  // For legacy reasons, we support the "factor" simple
  // form of a map. This is overridden as soon as a 
  // positive pair is added to the map.
  m_factor = 0;
}


//----------------------------------------------------------------
// Procedure: addPair

void DoubleMap::addPair(double x, double y)
{
  map<double, double> new_pos_mapping = m_pos_mapping;
  new_pos_mapping[x] = y;
  m_pos_mapping = new_pos_mapping;
}

//----------------------------------------------------------------
// Procedure: setFactor

void DoubleMap::setFactor(double value)
{
    m_factor = value;
}

//--------------------------------------------------------------------
// Procedure: setPoints     //inspired from handleThrustMap defined in USM_MOOSApp.cpp

bool DoubleMap::setPoints(string mapping) 
{
  if(mapping == "") {
    return(false);
  }
  else {
    bool ok = true;
    vector<string> svector = parseString(mapping, ',');
    unsigned int i, vsize = svector.size();
    for(i=0; i<vsize; i++) {
      string x = biteStringX(svector[i], ':');
      string y  = svector[i];
      if(isNumber(x) && isNumber(y))  {
        double dx = atof(x.c_str());
        double dy  = atof(y.c_str());
        addPair(dx, dy);
      }
      else {
        ok = false;
      }
    }
    if (ok)
      return(true);
    else
      return(false);
  }
}
  
//----------------------------------------------------------------
// Procedure: getYValue 

double DoubleMap::getYValue(double x) const
{
  if(x < 0)
    return(getYValueNeg(x));
  else if(x > 0)
    return(getYValuePos(x));
  else
    return(0);
}

//----------------------------------------------------------------
// Procedure: getXValue 

double DoubleMap::getXValue(double y) const
{
  if(y < 0)
    return(getXValueNeg(y));
  else if(y > 0)
    return(getXValuePos(y));
  else
    return(0);
}

//----------------------------------------------------------------
// Procedure: isAscending

bool DoubleMap::isAscending() const
{
  bool neg_ascending = isAscendingMap(m_neg_mapping);
  bool pos_ascending = isAscendingMap(m_pos_mapping);
  return(pos_ascending && neg_ascending);
}

//----------------------------------------------------------------
// Procedure: getFactor

double DoubleMap::getFactor() const
{
  return(m_factor);
}

//----------------------------------------------------------------
// Procedure: isAscendingMap

bool DoubleMap::isAscendingMap(map<double, double> gmap) const
{
  if(gmap.size() == 0)
    return(true);

  map<double, double>::const_iterator p;
  p = gmap.begin();
  double prev_range = p->second;
  p++;  
  while(p != gmap.end()) {
    double range  = p->second;
    if(range < prev_range) 
      return(false);
    prev_range = range;
    p++;
  }
  return(true);
}

//----------------------------------------------------------------
// Procedure: print

void DoubleMap::print() const
{
  cout << "Double Map: ----------------------------------------" << endl;
  map<double, double>::const_iterator p;
  p = m_neg_mapping.begin();
  while(p != m_neg_mapping.end()) {
    double x = p->first;
    double y = p->second;
    cout << "x: " << x << "  y: " << y << endl;
    p++;
  }

  p = m_pos_mapping.begin();
  while(p != m_pos_mapping.end()) {
    double x = p->first;
    double y = p->second;
    cout << "x: " << x << "  y: " << y << endl;
    p++;
  }

  m_figlog.print();
}

//----------------------------------------------------------------
// Procedure: clear

void DoubleMap::clear() 
{
  m_pos_mapping.clear();
  m_neg_mapping.clear();
  m_factor = 0;
  m_figlog.clear();
}

//----------------------------------------------------------------
// Procedure: usingFactor()
//      Note: The factor is used by default to calculated y
//            for a given x, unless/until a mapping is provided. 

bool DoubleMap::usingFactor() const
{
  if(m_pos_mapping.size() == 0)
    return(true);
  else
    return(false);
}

//----------------------------------------------------------------
// Procedure: getMapPos()

string DoubleMap::getMapPos() const
{
  string posmap;

  map<double, double>::const_iterator p;
  for(p=m_pos_mapping.begin(); p!=m_pos_mapping.end(); p++) {
    double x = p->first;
    double y  = p->second;
    if(posmap != "")
      posmap += ", ";
    posmap += doubleToStringX(x,4);
    posmap += ","+ doubleToStringX(y, 4);
  }
  return(posmap);
}

//----------------------------------------------------------------
// Procedure: getMapNeg()

string DoubleMap::getMapNeg() const
{
  if(m_reflect_negative)
    return("Positive thurst-map reflected");
  
  string negmap;
  map<double, double>::const_iterator p;
  for(p=m_neg_mapping.begin(); p!=m_neg_mapping.end(); p++) {
    double x = p->first;
    double y  = p->second;
    if(negmap != "")
      negmap += ", ";
    negmap += doubleToStringX(x,4);
    negmap += ":"+ doubleToStringX(y, 4);
  }
  return(negmap);
}


//----------------------------------------------------------------
// Procedure: getYValueNeg

double DoubleMap::getYValueNeg(double x) const
{
  if(m_neg_mapping.size() == 0) {
    if(m_reflect_negative)
      return(-1 * getYValuePos(-x));
    else
      return(0);
  }
  
  map<double, double>::const_iterator p = m_neg_mapping.begin();
  
  double left_dom = p->first;
  double left_val = p->second;

  if(x <= left_dom)
    return(left_val);

  double right_dom = 0;
  double right_val = 0;

  bool found = false;
  while((p != m_neg_mapping.end()) && !found) {
    double ix = p->first;
    double iy  = p->second;

    if(x <= ix) {
      right_dom = ix;
      right_val = iy;
      found = true;
    }
    else {
      left_dom = ix;
      left_val = iy;
    }    
    p++;
  }

  double run = (right_dom - left_dom);
  if(run <= 0)
    return(0);

  double rise  = (right_val - left_val);
  double slope = rise / run;

  double y_val = ((x-left_dom) * slope) + left_val;

  return(y_val);
}

//----------------------------------------------------------------
// Procedure: getYValuePos

double DoubleMap::getYValuePos(double x) const
{
  if(m_pos_mapping.size() == 0) {
    if(m_factor == 0)
      return(0);
    else
      return(x * m_factor);
  }
  
  map<double, double>::const_reverse_iterator p = m_pos_mapping.rbegin();
  
  double right_dom = p->first;
  double right_val = p->second;

  if(x >= right_dom)
    return(right_val);

  double left_dom = 0;
  double left_val = 0;

  bool found = false;
  while((p != m_pos_mapping.rend()) && !found) {
    double ix = p->first;
    double iy  = p->second;

    if(x >= ix) {
      left_dom = ix;
      left_val = iy;
      found = true;
    }
    else {
      right_dom = ix;
      right_val = iy;
    }    
    p++;
  }

  double run = (right_dom - left_dom);
  if(run <= 0) 
    return(0);

  double rise  = (right_val - left_val);
  double slope = rise / run;

  double y_val = ((x-left_dom) * slope) + left_val;

  return(y_val);
}

//----------------------------------------------------------------
// Procedure: getXValuePos

double DoubleMap::getXValuePos(double y) const
{
  if(m_pos_mapping.size() == 0) {
    if(m_factor == 0)
      return(0);
    else
      return(y * m_factor);
  }

  map<double, double>::const_reverse_iterator p = m_pos_mapping.rbegin();
  
  double right_dom = p->first;
  double right_val = p->second;

  if(y >= right_val)
    return(right_dom);

  double left_dom = 0;
  double left_val = 0;

  bool found = false;
  while((p != m_pos_mapping.rend()) && !found) {
    double ix = p->first;
    double iy  = p->second;

    if(y > iy) {
      left_dom = ix;
      left_val = iy;
      found = true;
    }
    else {
      right_dom = ix;
      right_val = iy;
    }    
    p++;
  }

  double run = (right_dom - left_dom);
  if(run <= 0) 
    return(0);

  double rise  = (right_val - left_val);
  double slope = rise / run;
  double b     = left_val - slope * left_dom;

  double x_val;
  if(slope == 0)
    x_val = left_dom;
  else
    x_val = (y - b) / slope;

  cout << "left_val:" << left_val;
  cout << "right_val:" << right_val;
  cout << "Slope: " << slope << endl;
  return(x_val);
}

//----------------------------------------------------------------
// Procedure: getXValueNeg

double DoubleMap::getXValueNeg(double y) const
{
  if(m_neg_mapping.size() == 0) {
    if(m_reflect_negative)
      return(-1 * getXValuePos(-y));
    else
      return(0);
  }
  
  map<double, double>::const_iterator p = m_neg_mapping.begin();
  
  double left_dom = p->first;
  double left_val = p->second;

  if(y <= left_val)
    return(left_dom);

  double right_dom = 0;
  double right_val = 0;

  bool found = false;
  while((p != m_neg_mapping.end()) && !found) {
    double ix = p->first;
    double iy  = p->second;

    if(y <= iy) {
      right_dom = ix;
      right_val = iy;
      found = true;
    }
    else {
      left_dom = ix;
      left_val = iy;
    }    
    p++;
  }

  double run = (right_dom - left_dom);
  if(run <= 0)
    return(0);

  double rise  = (right_val - left_val);
  double slope = rise / run;
  double b = left_val - slope * left_dom;

  
  double x_val = 0;
  if(slope == 0)
    x_val = left_dom;
  else
    x_val = (y - b) / slope;

  return(x_val);
}




