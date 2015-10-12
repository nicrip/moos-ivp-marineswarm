#--------------------------------------------------------
# The CMakeLists.txt for:                        lib_Ping
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

SET(SRC
  PingRecord.cpp
  PingContact.cpp
)

SET(HEADERS 
  PingRecord.h
  PingContact.h
) 

ADD_LIBRARY(Ping ${SRC})

TARGET_LINK_LIBRARIES(Ping
   mbutil
   behaviors
   geometry
   )
