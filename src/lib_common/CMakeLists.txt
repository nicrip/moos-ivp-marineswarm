##############################################################################
# FILE:        moos-ivp-extend/src/lib_common/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

## CppUnit
#include_directories(
#    ${CPPUNIT_INC}
#)
#link_directories(
#    ${CPPUNIT_LIB}
#)

#============================================================================
# List the subdirectories to build...
#============================================================================

# MOOSSafir common libs
ADD_SUBDIRECTORY(CoordinateSystem)
ADD_SUBDIRECTORY(Acoustic)
ADD_SUBDIRECTORY(Tests)
ADD_SUBDIRECTORY(Ping)
ADD_SUBDIRECTORY(SimMap)
ADD_SUBDIRECTORY(Node)
ADD_SUBDIRECTORY(Current)
ADD_SUBDIRECTORY(CoupleOfContact)
ADD_SUBDIRECTORY(Geometry)
ADD_SUBDIRECTORY(Hungarian)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
