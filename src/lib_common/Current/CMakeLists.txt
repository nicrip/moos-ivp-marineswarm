#--------------------------------------------------------
# The CMakeLists.txt for:                     lib_Current
# Author(s):             Nathan Vandervaeren, Simon Rohou
#--------------------------------------------------------

SET(SRC
  ConstantCurrent.cpp
  CurrentModel.cpp
  LangmuirCurrent.cpp
  RandomCurrent.cpp
  ShearCurrent.cpp
  SolitonCurrent.cpp
  SpectralCurrent.cpp
  MultiSpectralCurrent.cpp
  VortexCurrent.cpp
  GridCurrent.cpp
)

SET(HEADERS 
  ConstantCurrent.h
  CurrentModel.h
  LangmuirCurrent.h
  RandomCurrent.h
  ShearCurrent.h
  SolitonCurrent.h
  SpectralCurrent.h
  MultiSpectralCurrent.h
  VortexCurrent.h
  GridCurrent.h
) 

ADD_LIBRARY(Current ${SRC})

TARGET_LINK_LIBRARIES(Current
   behaviors
   geometry
   mbutil
   )
