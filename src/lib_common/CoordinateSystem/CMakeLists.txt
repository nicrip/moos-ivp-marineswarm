#--------------------------------------------------------
# The CMakeLists.txt for:            lib_CoordinateSystem
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  SphericalCoordinateSystem.cpp
)

SET(HEADERS 
  SphericalCoordinateSystem.h
) 

ADD_LIBRARY(CoordinateSystem ${SRC})

TARGET_LINK_LIBRARIES(CoordinateSystem
   mbutil
   behaviors
   geometry
   )
