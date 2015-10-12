#--------------------------------------------------------
# The CMakeLists.txt for:                    lib_Acoustic
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  SoundPropagationInWater.cpp
)

SET(HEADERS 
  SoundPropagationInWater.h
) 

ADD_LIBRARY(Acoustic ${SRC})

TARGET_LINK_LIBRARIES(Acoustic
   mbutil
   behaviors
   geometry
   )
