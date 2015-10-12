
#ifndef SPEEDTHRUST_MAP_HEADER
#define SPEEDTHRUST_MAP_HEADER

#include <string>
#include <vector>
#include <map>
#include "Figlog.h"
#include "DoubleMap.h"

class ThrustMap : public DoubleMap
{
public:
  ThrustMap();
  ~ThrustMap() {};
  
 public: // Special Setters
  void   setThrustFactor(double value);
 
 public: // Special Getters
  double getThrustValue(double speed) const;
  double getSpeedValue(double thrust) const;
  double getThrustFactor() const;
  bool   usingThrustFactor() const;

};

#endif 


