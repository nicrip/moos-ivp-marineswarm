/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ThrustMap.h                                          */
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

#ifndef DOUBLE_MAP_HEADER
#define DOUBLE_MAP_HEADER

#include <string>
#include <vector>
#include <map>
#include "Figlog.h"

class DoubleMap
{
public:
  DoubleMap();
  ~DoubleMap() {};

 public: // Setters
  void   addPair(double, double);
  void   setFactor(double);
  void   setReflect(bool v)  {m_reflect_negative=v;};
  bool   setPoints(std::string);           //added by Nathan
 
 public: // Getters
  double getYValue(double x) const;
  double getXValue(double y) const;
  double getFactor() const;
  bool   isAscending() const;
  bool   isConfigured();
  bool   usingFactor() const;

  std::string getMapPos() const;
  std::string getMapNeg() const;

 public: // Actions
  void   print() const;
  void   clear();
  void   clearFiglog() {m_figlog.clear();};

 protected:
  bool   isAscendingMap(std::map<double, double>) const;
  double getYValueNeg(double x) const;
  double getYValuePos(double x) const;
  double getXValueNeg(double y) const;
  double getXValuePos(double y) const;

 protected:
  std::map<double, double> m_pos_mapping;
  std::map<double, double> m_neg_mapping;

  bool   m_reflect_negative;

  double m_factor;

  Figlog m_figlog;
};

#endif 


