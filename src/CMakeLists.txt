##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################

#============================================================================
# Let users control whether or not GUI-related libraries/apps are built...
#============================================================================

IF(BUILD_GUI_CODE)
	SET(MOOSSAFIR_BUILD_GUI_CODE TRUE)
ELSE()
	SET(MOOSSAFIR_BUILD_GUI_CODE FALSE)
ENDIF()

# clean is needed


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})


#===============================================================================
# FINDING MOOSGeodesy' HEADERS AND LIBRARIES...
#===============================================================================

# moos - http://www.robots.ox.ac.uk/~mobile/MOOS/wiki/pmwiki.php
find_package(MOOSGeodesy)
include_directories(${MOOSGeodesy_INCLUDE_DIRS})
link_directories(${MOOSGeodesy_LIBRARY_PATH})

message("+++++++++++++++++++++++++++++++++++++++++")
message("MOOSGeodesy_INCLUDE_DIRS:" ${MOOSGeodesy_INCLUDE_DIRS})
message("+++++++++++++++++++++++++++++++++++++++++")

#============================================================================
# List the subdirectories to build...
#============================================================================

# MOOSSafir common libs
ADD_SUBDIRECTORY(lib_common)

# MOOSSafir behavior libs
ADD_SUBDIRECTORY(lib_behaviors)


# MOOSSafir comms messages lib
ADD_SUBDIRECTORY(lib_moossafir_protobuf)


# MOOSSafir apps
ADD_SUBDIRECTORY(uFldPingManager_CGG)
# cannot find Geodesy files in v14.x: ADD_SUBDIRECTORY(uSimMarine_CGG)
ADD_SUBDIRECTORY(pMarinePIDFreedom_CGG)
ADD_SUBDIRECTORY(uSimMarineFreedom_CGG)
ADD_SUBDIRECTORY(uSimCurrent_CGG)
ADD_SUBDIRECTORY(uSimConsumption_CGG)
ADD_SUBDIRECTORY(uSimDeployment_CGG)
ADD_SUBDIRECTORY(uSimShot_CGG)
ADD_SUBDIRECTORY(pContactsViewer_CGG)
ADD_SUBDIRECTORY(uSimFolaga_CGG)
ADD_SUBDIRECTORY(pFolagaPID_CGG)
ADD_SUBDIRECTORY(pLogger_CGG)
#ADD_SUBDIRECTORY(iMSEASOceanModelDirect)
ADD_SUBDIRECTORY(pTrailViewer)
ADD_SUBDIRECTORY(pFormationViewer)
ADD_SUBDIRECTORY(pFormationQualityMetric)
ADD_SUBDIRECTORY(pNodeLogger)
ADD_SUBDIRECTORY(pShoreLogger)
#ADD_SUBDIRECTORY(pSimulateNodeLoss)
#ADD_SUBDIRECTORY(pThreeChannelCurrent)
#ADD_SUBDIRECTORY(pIrrotationalVortexCurrent)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
