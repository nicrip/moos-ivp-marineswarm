#--------------------------------------------------------
# The CMakeLists.txt for:          iMSEASOceanModelDirect
# Author(s):       			     Nick Rypkema                       
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

include_directories (${Octave_INCLUDE_DIR})

include_directories (${Octave_INCLUDE_DIR}/octave)

SET(SRC
  MSEASOceanModelDirect.cpp
  MSEASOceanModelDirect_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(iMSEASOceanModelDirect ${SRC})

TARGET_LINK_LIBRARIES(iMSEASOceanModelDirect
   ${Octave_LIBRARIES}
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread
   contacts)
