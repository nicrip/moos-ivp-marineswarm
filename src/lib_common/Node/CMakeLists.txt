#--------------------------------------------------------
# The CMakeLists.txt for:                        lib_Node
# Author(s):                          Nathan Vandervaeren
#--------------------------------------------------------

SET(SRC
  NodeRecord6DOF.cpp
  NodeRecord6DOFUtils.cpp
)

SET(HEADERS 
  NodeRecord6DOF.h
  NodeRecord6DOFUtils.h
) 

ADD_LIBRARY(Node ${SRC})

TARGET_LINK_LIBRARIES(Node 
  ${MOOS_LIBRARIES}
  contacts
)
