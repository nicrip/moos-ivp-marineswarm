#include <iostream>
#include "Geometry/IncrementalConvexHull.h"

int main()
{
  IncrementalConvexHull ch;
  ch.addPoint(0,0);
  ch.addPoint(100,0);
  ch.addPoint(100,100);
  ch.printPoints();
  ch.addPoint(-5,-5);
  ch.printPoints();
  std::cout << "Hello World!" << std::endl;
}
