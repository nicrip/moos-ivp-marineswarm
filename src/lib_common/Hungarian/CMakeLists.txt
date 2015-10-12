#--------------------------------------------------------
# The CMakeLists.txt for:       lib_IncrementalConvexHull
# Author(s):                                 Nick Rypkema
#--------------------------------------------------------

SET(SRC
  hungarian.cpp
)

SET(HEADERS 
  hungarian.h
) 

ADD_LIBRARY(Hungarian ${SRC})

TARGET_LINK_LIBRARIES(Hungarian
   mbutil
   behaviors
   geometry
   )
