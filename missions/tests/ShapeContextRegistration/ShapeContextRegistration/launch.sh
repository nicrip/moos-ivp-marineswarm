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
START_POS1="0,0"    
START_POS2="80,0"
START_POS3="160,0"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS1="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS2="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS3="$x_number,$y_number"
DRIFT_POS_X1=100
DRIFT_POS_Y1=-100
DRIFT_POS_X2=200
DRIFT_POS_Y2=-100
DRIFT_POS_X3=300
DRIFT_POS_Y3=-100
DRIFT_UPDATES1="DRIFT_UPDATES1"
DRIFT_UPDATES2="DRIFT_UPDATES2"
DRIFT_UPDATES2="DRIFT_UPDATES3"
MODEM_ID1=1
MODEM_ID2=2
MODEM_ID3=3

SHORE_LISTEN="9300"

nsplug meta_vehicle.moos targ_$VNAME1.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME1      START_POS=$START_POS1                  \
    VPORT="9001"       SHARE_LISTEN="9301"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID1

nsplug meta_vehicle.moos targ_$VNAME2.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME2      START_POS=$START_POS2                  \
    VPORT="9002"       SHARE_LISTEN="9302"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID2

nsplug meta_vehicle.moos targ_$VNAME3.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME3      START_POS=$START_POS3                  \
    VPORT="9003"       SHARE_LISTEN="9303"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID3

nsplug meta_vehicle.bhv targ_$VNAME1.bhv -f VNAME=$VNAME1     \
    DRIFT_POS_X=$DRIFT_POS_X1 DRIFT_POS_Y=$DRIFT_POS_Y1 CONTACT="" DRIFT_UPDATES=$DRIFT_UPDATES1

nsplug meta_vehicle.bhv targ_$VNAME2.bhv -f VNAME=$VNAME2     \
    DRIFT_POS_X=$DRIFT_POS_X2 DRIFT_POS_Y=$DRIFT_POS_Y2 CONTACT="" DRIFT_UPDATES=$DRIFT_UPDATES2

nsplug meta_vehicle.bhv targ_$VNAME3.bhv -f VNAME=$VNAME3     \
    DRIFT_POS_X=$DRIFT_POS_X3 DRIFT_POS_Y=$DRIFT_POS_Y3 CONTACT="" DRIFT_UPDATES=$DRIFT_UPDATES3

nsplug meta_shoreside.moos targ_shoreside.moos -f WARP=$TIME_WARP \
   VNAME="shoreside"  SHARE_LISTEN=$SHORE_LISTEN  VPORT="9000" DRIFT_UPDATES1=$DRIFT_UPDATES1 DRIFT_UPDATES2=$DRIFT_UPDATES2
        
if [ ${JUST_MAKE} = "yes" ] ; then
    exit 0
fi

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
printf "Done \n"

#-----------------------------------------------------------
#  Part 4: Launch uMAC and kill everything upon exiting uMAC
#-----------------------------------------------------------
uMAC targ_shoreside.moos
printf "Killing all processes ... \n"
kill %1 %2 %3 %4
printf "Done killing processes.   \n"
