##############################################################################
# FILE:        moos-ivp-extend/src/lib_behaviors/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_COMMON_LIBRARY_DIRS ./lib_common/lib_*)
FILE(GLOB LOCAL_BHV_LIBRARY_DIRS ./lib_behaviors/lib_*)
INCLUDE_DIRECTORIES(${LOCAL_COMMON_LIBRARY_DIRS} ${LOCAL_BHV_LIBRARY_DIRS})

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

# MOOSSafir behavior libs
ADD_SUBDIRECTORY(Formation_Behavior)
ADD_SUBDIRECTORY(ContactsPing_Formation)
ADD_SUBDIRECTORY(OneContact_PingFormation)
ADD_SUBDIRECTORY(Map_PingFormation)
ADD_SUBDIRECTORY(Isosceles_PingFormation)
ADD_SUBDIRECTORY(TrailiSBL_PingFormation)
ADD_SUBDIRECTORY(Relax_PingFormation)
ADD_SUBDIRECTORY(Pairs_MapPingFormation)
ADD_SUBDIRECTORY(VirtualStructure_MapPingFormation)
ADD_SUBDIRECTORY(Trail_PingFormation)

ADD_SUBDIRECTORY(DriftingTarget)
ADD_SUBDIRECTORY(DriftingTargetWaypoint)
ADD_SUBDIRECTORY(ManageAcousticPing)
ADD_SUBDIRECTORY(DriftingTgtWptContacts)
ADD_SUBDIRECTORY(AcousticPingPlanner)
ADD_SUBDIRECTORY(PairwiseNeighbourReferencing)
ADD_SUBDIRECTORY(PairwiseNeighbourReferencingOpt)
ADD_SUBDIRECTORY(DriftingForce)
ADD_SUBDIRECTORY(DriftingForceWaypoint)
ADD_SUBDIRECTORY(AttractionRepulsion)
ADD_SUBDIRECTORY(AttractionRepulsionReferencing)
ADD_SUBDIRECTORY(RigidNeighbourRegistration)
ADD_SUBDIRECTORY(AttractionRepulsionOpt)
ADD_SUBDIRECTORY(AttractionRepulsionReferencingOpt)
ADD_SUBDIRECTORY(ShapeContextRegistration)
ADD_SUBDIRECTORY(AssignmentRegistration)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################

