#--------------------------------------------------------
# The CMakeLists.txt for:              Formation_Behavior
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
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                      Formation_Behavior
#--------------------------------------------------------
ADD_LIBRARY(Formation_Behavior SHARED 
   Formation_Behavior.cpp)
TARGET_LINK_LIBRARIES(Formation_Behavior
   helmivp
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
   # MOOSSafir libraries:
   ${SYSTEM_LIBS} )
