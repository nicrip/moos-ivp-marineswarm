#--------------------------------------------------------
# The CMakeLists.txt for:                      lib_SimMap
# Author(s):                          Nathan Vandervaeren
#--------------------------------------------------------

SET(SRC
  DoubleMap.cpp
  DragMap.cpp
  ThrustMap.cpp
)

SET(HEADERS 
  DoubleMap.h
  DragMap.h
  ThrustMap.h
) 

ADD_LIBRARY(SimMap ${SRC})

TARGET_LINK_LIBRARIES(SimMap
   mbutil
   geometry
   )
