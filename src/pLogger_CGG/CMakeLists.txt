#--------------------------------------------------------
# The CMakeLists.txt for:                     pLogger_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  Logger_CGG.cpp
  Logger_CGG_Info.cpp
  Logger_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(pLogger_CGG ${SRC})

TARGET_LINK_LIBRARIES(pLogger_CGG
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
