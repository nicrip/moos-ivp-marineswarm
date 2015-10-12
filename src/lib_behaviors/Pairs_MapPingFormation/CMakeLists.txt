#--------------------------------------------------------
# The CMakeLists.txt for:          Pairs_MapPingFormation
# Author(s):                                  Simon Rohou
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
    Map_PingFormation
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                  Pairs_MapPingFormation
#--------------------------------------------------------
ADD_LIBRARY(BHV_Pairs_MapPingFormation SHARED 
   Pairs_MapPingFormation.cpp)
TARGET_LINK_LIBRARIES(BHV_Pairs_MapPingFormation
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
   # MOOSSafir libraries:
   Map_PingFormation
   ${SYSTEM_LIBS} )
