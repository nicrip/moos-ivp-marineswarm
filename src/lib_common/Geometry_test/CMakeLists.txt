#--------------------------------------------------------
# The CMakeLists.txt for:       lib_IncrementalConvexHull
# Author(s):                                 Nick Rypkema
#--------------------------------------------------------

SET(SRC
  test.cpp
)

ADD_EXECUTABLE(test ${SRC})

TARGET_LINK_LIBRARIES(test
   geometry
   mbutil
   behaviors
   IncrementalConvexHull
   )
