#--------------------------------------------------------
# The CMakeLists.txt for:              uSimDeployment_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SimDeployment_CGG.cpp
  SimDeployment_CGG_Info.cpp
  SimDeployment_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(uSimDeployment_CGG ${SRC})

TARGET_LINK_LIBRARIES(uSimDeployment_CGG
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
