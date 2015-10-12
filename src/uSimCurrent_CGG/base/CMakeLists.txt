#--------------------------------------------------------
# The CMakeLists.txt for:                 uSimCurrent_CGG
# Author(s):             Nathan Vandervaeren, Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  CurrentMOOSApp.cpp
  CurrentMOOSApp_Info.cpp
  CurrentMOOSApp_Test.cpp
  main.cpp
)

SET(CMAKE_CXX_FLAGS "-std=c++0x")

ADD_EXECUTABLE(uSimCurrent_CGG ${SRC})

TARGET_LINK_LIBRARIES(uSimCurrent_CGG
  ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil
  # MOOSSafir libraries:
  Current
  Tests
  ${SYSTEM_LIBS})
