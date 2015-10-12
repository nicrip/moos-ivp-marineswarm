#--------------------------------------------------------
# The CMakeLists.txt for:                       lib_Tests
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  UnitTests.cpp
)

SET(HEADERS 
  UnitTests.h
) 

ADD_LIBRARY(Tests ${SRC})

TARGET_LINK_LIBRARIES(Tests
   behaviors
   geometry
   mbutil
   )
