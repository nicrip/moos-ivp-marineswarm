#--------------------------------------------------------
# The CMakeLists.txt for:                       pShoreLogger
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  ShoreLogger.cpp
  ShoreLogger_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pShoreLogger ${SRC})

TARGET_LINK_LIBRARIES(pShoreLogger
   ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil)

