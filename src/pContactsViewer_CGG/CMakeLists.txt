#--------------------------------------------------------
# The CMakeLists.txt for:             pContactsViewer_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  ContactsViewer_CGG.cpp
  ContactsViewer_CGG_Info.cpp
  ContactsViewer_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(pContactsViewer_CGG ${SRC})

TARGET_LINK_LIBRARIES(pContactsViewer_CGG
  ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil
  ${SYSTEM_LIBS})