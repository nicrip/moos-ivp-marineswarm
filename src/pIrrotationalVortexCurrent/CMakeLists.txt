#--------------------------------------------------------
# The CMakeLists.txt for:                       pIrrotationalVortexCurrent
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  IrrotationalVortexCurrent.cpp
  IrrotationalVortexCurrent_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pIrrotationalVortexCurrent ${SRC})

TARGET_LINK_LIBRARIES(pIrrotationalVortexCurrent
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

