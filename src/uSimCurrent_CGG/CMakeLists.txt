##############################################################################
# FILE:        moos-ivp-extend/src/uSimCurrent_CGG/CMakeLists.txt
# DATE:        2014/04/30
# DESCRIPTION: CMakeLists.txt file
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

#============================================================================
# List the subdirectories to build...
#============================================================================

# MOOSSafir app uSimCurrent_CGG

IF(MOOSSAFIR_BUILD_GUI_CODE)
  ADD_SUBDIRECTORY(base_and_viewer)
ELSE(MOOSSAFIR_BUILD_GUI_CODE)
  ADD_SUBDIRECTORY(base)
ENDIF(MOOSSAFIR_BUILD_GUI_CODE)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################