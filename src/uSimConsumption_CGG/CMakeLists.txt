#--------------------------------------------------------
# The CMakeLists.txt for:             uSimConsumption_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SimConsumption_CGG.cpp
  SimConsumption_CGG_Info.cpp
  SimConsumption_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(uSimConsumption_CGG ${SRC})

TARGET_LINK_LIBRARIES(uSimConsumption_CGG
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
