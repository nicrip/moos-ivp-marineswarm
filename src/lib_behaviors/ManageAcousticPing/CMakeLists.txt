#--------------------------------------------------------
# The CMakeLists.txt for:          BHV_ManageAcousticPing
# Author(s):                                 Nick Rypkema
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  # Windows Libraries
  SET(SYSTEM_LIBS
      )
else (${WIN32})
  # Linux and Apple Libraries
  SET(SYSTEM_LIBS
      m )
endif (${WIN32})


MACRO(ADD_BHV BHV_NAME)
  ADD_LIBRARY(${BHV_NAME} SHARED ${BHV_NAME}.cpp)
  TARGET_LINK_LIBRARIES(${BHV_NAME}
    helmivp
    behaviors
    ivpbuild
    logic
    ivpcore
    bhvutil
    mbutil
    geometry
    # MOOSSafir libraries:
    CoordinateSystem
    Ping
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                      BHV_DriftingTarget
#--------------------------------------------------------
ADD_LIBRARY(ManageAcousticPing SHARED
   BHV_ManageAcousticPing.cpp)
TARGET_LINK_LIBRARIES(ManageAcousticPing
   helmivp
   behaviors
   ivpbuild
   logic
   ivpcore
   bhvutil
   mbutil
   geometry
   # MOOSSafir libraries:
   CoordinateSystem
   Ping
   ${SYSTEM_LIBS} )
