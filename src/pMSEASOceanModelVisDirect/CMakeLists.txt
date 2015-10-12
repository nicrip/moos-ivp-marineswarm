#--------------------------------------------------------
# The CMakeLists.txt for:                       pMSEASOceanModelVisDirect
# Author(s):                              
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

include_directories (${Octave_INCLUDE_DIR})

include_directories (${Octave_INCLUDE_DIR}/octave)

SET(SRC
  MSEASOceanModelVisDirect.cpp
  MSEASOceanModelVisDirect_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pMSEASOceanModelVisDirect ${SRC})

TARGET_LINK_LIBRARIES(pMSEASOceanModelVisDirect
   ${Octave_LIBRARIES}
   ${MOOS_LIBRARIES}
   MOOSGeodesy
   proj
   geometry
   mbutil
   m
   pthread)

