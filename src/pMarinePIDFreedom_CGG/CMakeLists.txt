#--------------------------------------------------------
# The CMakeLists.txt for:           pMarinePIDFreedom_CGG
# Author(s):                          Nathan Vandervaeren
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  SET(SYSTEM_LIBS
    wsock32)
else (${WIN32})
  SET(SYSTEM_LIBS
    m
    pthread)
endif (${WIN32})

SET(SRC
   MarinePID.cpp
   MarinePID_Info.cpp
   PIDEngine.cpp
   ScalarPID.cpp
   main.cpp
)

ADD_EXECUTABLE(pMarinePIDFreedom_CGG ${SRC})
   
TARGET_LINK_LIBRARIES(pMarinePIDFreedom_CGG 
  ${MOOS_LIBRARIES}
  mbutil
  geometry 
  # MOOSSafir libraries:
  SimMap
  ${SYSTEM_LIBS})

