#--------------------------------------------------------
# The CMakeLists.txt for:                       pThreeChannelCurrent
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  ThreeChannelCurrent.cpp
  ThreeChannelCurrent_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pThreeChannelCurrent ${SRC})

TARGET_LINK_LIBRARIES(pThreeChannelCurrent
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

