#--------------------------------------------------------
# The CMakeLists.txt for:        OneContact_PingFormation
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
    ContactsPing_Formation
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                OneContact_PingFormation
#--------------------------------------------------------
ADD_LIBRARY(OneContact_PingFormation SHARED 
   OneContact_PingFormation.cpp)
TARGET_LINK_LIBRARIES(OneContact_PingFormation
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
   # MOOSSafir libraries:
   ContactsPing_Formation
   ${SYSTEM_LIBS} )
