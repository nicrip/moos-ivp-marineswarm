#--------------------------------------------------------
# The CMakeLists.txt for:                       pTrailViewer
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  TrailViewer.cpp
  TrailViewer_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pTrailViewer ${SRC})

TARGET_LINK_LIBRARIES(pTrailViewer
   ${MOOS_LIBRARIES}
   m
   pthread
   apputil
   geometry 
   mbutil)

