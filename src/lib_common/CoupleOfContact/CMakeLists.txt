#--------------------------------------------------------
# The CMakeLists.txt for:             lib_CoupleOfContact
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  CoupleOfContact.cpp
)

SET(HEADERS 
  CoupleOfContact.h
) 

ADD_LIBRARY(CoupleOfContact ${SRC})

TARGET_LINK_LIBRARIES(CoupleOfContact
   mbutil
   behaviors
   geometry
   )
