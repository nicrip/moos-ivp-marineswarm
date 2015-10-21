#--------------------------------------------------------
# The CMakeLists.txt for:                       pFormationQualityMetric
# Author(s):                              Nick Rypkema
#--------------------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
  FormationQualityMetric.cpp
  FormationQualityMetric_Info.cpp
  main.cpp
)



ADD_EXECUTABLE(pFormationQualityMetric ${SRC})

TARGET_LINK_LIBRARIES(pFormationQualityMetric
   ${MOOS_LIBRARIES}
   geometry
   mbutil
   m
   pthread
   armadillo
   #openblas
   blas
   contacts)

