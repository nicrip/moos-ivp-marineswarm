#--------------------------------------------------------
# The CMakeLists.txt for:       lib_IncrementalConvexHull
# Author(s):                                 Nick Rypkema
#--------------------------------------------------------

SET(SRC
  IncrementalConvexHull.cpp
)

SET(HEADERS 
  IncrementalConvexHull.h
) 

ADD_LIBRARY(IncrementalConvexHull ${SRC})

TARGET_LINK_LIBRARIES(IncrementalConvexHull
   mbutil
   behaviors
   geometry
   )
