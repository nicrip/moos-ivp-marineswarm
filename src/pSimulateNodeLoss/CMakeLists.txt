#--------------------------------------------------------
# The CMakeLists.txt for:                       pSimulateNodeLoss
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  SimulateNodeLoss.cpp
  SimulateNodeLoss_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pSimulateNodeLoss ${SRC})

TARGET_LINK_LIBRARIES(pSimulateNodeLoss
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

