#---------------------------------------------------------
# The CMakeLists.txt for:       BHV_AttractionRepulsionOpt
# Author(s):                                  Nick Rypkema
#---------------------------------------------------------

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
    nlopt
    # MOOSSafir libraries:
    DriftingTarget
    ManageAcousticPing
    AcousticPingPlanner
    ${SYSTEM_LIBS} )
ENDMACRO(ADD_BHV)


#--------------------------------------------------------
#                                 BHV_AttractionRepulsion
#--------------------------------------------------------
ADD_LIBRARY(BHV_AttractionRepulsionReferencingOpt SHARED
   BHV_AttractionRepulsionReferencingOpt.cpp)
TARGET_LINK_LIBRARIES(BHV_AttractionRepulsionReferencingOpt
   helmivp
   behaviors
   ivpbuild
   logic
   ivpcore
   bhvutil
   mbutil
   geometry
   nlopt
   # MOOSSafir libraries:
   DriftingTarget
   ManageAcousticPing
   AcousticPingPlanner
   ${SYSTEM_LIBS} )
