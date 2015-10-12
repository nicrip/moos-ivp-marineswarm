#!/bin/bash 
#-----------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-----------------------------------------------------------
TIME_WARP=1
JUST_MAKE="no"
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [SWITCHES] [time_warp]   \n" $0
	printf "  --just_make, -j    \n" 
	printf "  --help, -h         \n" 
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
	JUST_MAKE="yes"
    else 
	printf "Bad Argument: %s \n" $ARGI
	exit 0
    fi
done

#-----------------------------------------------------------
#  Part 2: Create the .moos and .bhv files. 
#-----------------------------------------------------------
VNAME1="alpha"  
VNAME2="bravo"
VNAME3="charlie"
VNAME4="delta"
VNAME5="echo"
VNAME6="foxtrot"
VNAME7="golf"
VNAME8="hotel"
VNAME9="india"

#-----------------------------------------------------------
#  Part 3: Launch the processes
#-----------------------------------------------------------
printf "Launching $SNAME MOOS Community (WARP=%s) \n"  $TIME_WARP
pAntler targ_shoreside.moos >& /dev/null &
printf "Launching $VNAME1 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME1.moos >& /dev/null &
printf "Launching $VNAME2 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME2.moos >& /dev/null &
printf "Launching $VNAME3 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME3.moos >& /dev/null &
printf "Launching $VNAME4 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME4.moos >& /dev/null &
printf "Launching $VNAME5 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME5.moos >& /dev/null &
printf "Launching $VNAME6 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME6.moos >& /dev/null &
printf "Launching $VNAME7 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME7.moos >& /dev/null &
printf "Done \n"

#-----------------------------------------------------------
#  Part 4: Launch uMAC and kill everything upon exiting uMAC
#-----------------------------------------------------------
uMAC targ_shoreside.moos
printf "Killing all processes ... \n"
kill %1 %2 %3 %4 %5 %6 %7 %8
printf "Done killing processes.   \n"
