#--------------------------------------------------------
# The CMakeLists.txt for:                    uSimShot_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SimShot_CGG.cpp
  SimShot_CGG_Info.cpp
  SimShot_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(uSimShot_CGG ${SRC})

TARGET_LINK_LIBRARIES(uSimShot_CGG
  ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil
  # MOOSSafir libraries:
  ${SYSTEM_LIBS})
