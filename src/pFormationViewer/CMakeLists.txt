#--------------------------------------------------------
# The CMakeLists.txt for:                       pFormationViewer
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  FormationViewer.cpp
  FormationViewer_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pFormationViewer ${SRC})

TARGET_LINK_LIBRARIES(pFormationViewer
   ${MOOS_LIBRARIES}
   m
   pthread
   apputil
   geometry 
   mbutil
   contacts)
