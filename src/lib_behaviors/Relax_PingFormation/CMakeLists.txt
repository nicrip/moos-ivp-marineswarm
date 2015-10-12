#--------------------------------------------------------
# The CMakeLists.txt for:             Relax_PingFormation
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
    OneContact_PingFormation
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                     Relax_PingFormation
#--------------------------------------------------------
ADD_LIBRARY(BHV_Relax_PingFormation SHARED 
   Relax_PingFormation.cpp)
TARGET_LINK_LIBRARIES(BHV_Relax_PingFormation
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
   # MOOSSafir libraries:
   OneContact_PingFormation
   ${SYSTEM_LIBS} )
