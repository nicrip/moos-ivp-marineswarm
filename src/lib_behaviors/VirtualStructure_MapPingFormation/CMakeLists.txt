#--------------------------------------------------------
# The CMakeLists.txt for:  VirtualStructure_MapPingFormation
# Author(s):                          Nathan Vandervaeren
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
    armadillo
    # MOOSSafir libraries:
    BHV_Map_PingFormation
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                       VirtualStructure_MapPingFormation
#--------------------------------------------------------
ADD_LIBRARY(BHV_VirtualStructure_MapPingFormation SHARED 
   VirtualStructure_MapPingFormation.cpp)
TARGET_LINK_LIBRARIES(BHV_VirtualStructure_MapPingFormation
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry
   armadillo 
   # MOOSSafir libraries:
   Map_PingFormation
   ${SYSTEM_LIBS} )
