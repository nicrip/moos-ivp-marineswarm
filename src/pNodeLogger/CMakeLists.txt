#--------------------------------------------------------
# The CMakeLists.txt for:                       pNodeLogger
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  NodeLogger.cpp
  NodeLogger_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pNodeLogger ${SRC})

TARGET_LINK_LIBRARIES(pNodeLogger
   ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil)

