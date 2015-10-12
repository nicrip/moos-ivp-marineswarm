#--------------------------------------------------------
# The CMakeLists.txt for:                  uSimFolaga_CGG
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
   USM_MOOSApp.cpp
   USM_Model.cpp
   USM_Info.cpp
   SimEngine.cpp
   main.cpp
)

ADD_EXECUTABLE(uSimFolaga_CGG ${SRC})

TARGET_LINK_LIBRARIES(uSimFolaga_CGG 
  ${MOOS_LIBRARIES}
  MOOSGeodesy
  contacts
  geometry 
  apputil
  mbutil
  # MOOSSafir libraries:
  SimMap
  ${SYSTEM_LIBS})


