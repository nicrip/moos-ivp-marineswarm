
#ifndef SPEEDDRAG_MAP_HEADER
#define SPEEDDRAG_MAP_HEADER

#include <string>
#include <vector>
#include <map>
#include "Figlog.h"
#include "DoubleMap.h"

class DragMap : public DoubleMap
{
public:
  DragMap();
  ~DragMap() {};
  
 public: // Special Setters
  void   setDragFactor(double value);
 
 public: // Special Getters
  double getDragValue(double speed) const;
  double getSpeedValue(double drag) const;
  double getDragFactor() const;
  bool   usingDragFactor() const;

};

#endif 


