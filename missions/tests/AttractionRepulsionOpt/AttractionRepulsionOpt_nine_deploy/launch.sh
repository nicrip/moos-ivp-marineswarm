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
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS4="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS5="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS6="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS7="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS8="$x_number,$y_number"
x_number=$(shuf -i 0-200 -n 1)
x_number=$((x_number-100))
y_number=$(shuf -i 0-200 -n 1)
y_number=$((y_number-100)) 
START_POS9="$x_number,$y_number"
START_POS1="0,0"
START_POS2="100,0"
START_POS3="-100,0"
START_POS4="50,100"
START_POS5="-50,100"
START_POS6="0,200"
START_POS7="-50,-100"
START_POS8="50,-100"
START_POS9="0,-200"
MODEM_ID1=1
MODEM_ID2=2
MODEM_ID3=3
MODEM_ID4=4
MODEM_ID5=5
MODEM_ID6=6
MODEM_ID7=7
MODEM_ID8=8
MODEM_ID9=9

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

nsplug meta_vehicle.moos targ_$VNAME4.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME4      START_POS=$START_POS4                  \
    VPORT="9004"       SHARE_LISTEN="9304"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID4

nsplug meta_vehicle.moos targ_$VNAME5.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME5      START_POS=$START_POS5                  \
    VPORT="9005"       SHARE_LISTEN="9305"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID5

nsplug meta_vehicle.moos targ_$VNAME6.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME6      START_POS=$START_POS6                  \
    VPORT="9006"       SHARE_LISTEN="9306"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID6

nsplug meta_vehicle.moos targ_$VNAME7.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME7      START_POS=$START_POS7                  \
    VPORT="9007"       SHARE_LISTEN="9307"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID7

nsplug meta_vehicle.moos targ_$VNAME8.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME8      START_POS=$START_POS8                  \
    VPORT="9008"       SHARE_LISTEN="9308"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID8

nsplug meta_vehicle.moos targ_$VNAME9.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME9      START_POS=$START_POS9                  \
    VPORT="9009"       SHARE_LISTEN="9309"                    \
    VTYPE="kayak"      SHORE_LISTEN=$SHORE_LISTEN	MODEM_ID=$MODEM_ID9

nsplug meta_vehicle.bhv targ_$VNAME1.bhv -f VNAME=$VNAME1     \
    

nsplug meta_vehicle.bhv targ_$VNAME2.bhv -f VNAME=$VNAME2     \
    

nsplug meta_vehicle.bhv targ_$VNAME3.bhv -f VNAME=$VNAME3     \
    

nsplug meta_vehicle.bhv targ_$VNAME4.bhv -f VNAME=$VNAME4     \
    

nsplug meta_vehicle.bhv targ_$VNAME5.bhv -f VNAME=$VNAME5     \
    

nsplug meta_vehicle.bhv targ_$VNAME6.bhv -f VNAME=$VNAME6     \
    

nsplug meta_vehicle.bhv targ_$VNAME7.bhv -f VNAME=$VNAME7     \
    

nsplug meta_vehicle.bhv targ_$VNAME8.bhv -f VNAME=$VNAME8     \
    

nsplug meta_vehicle.bhv targ_$VNAME9.bhv -f VNAME=$VNAME9     \


nsplug meta_shoreside.moos targ_shoreside.moos -f WARP=$TIME_WARP \
   VNAME="shoreside"  SHARE_LISTEN=$SHORE_LISTEN  VPORT="9000"
        
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
printf "Launching $VNAME4 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME4.moos >& /dev/null &
printf "Launching $VNAME5 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME5.moos >& /dev/null &
printf "Launching $VNAME6 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME6.moos >& /dev/null &
printf "Launching $VNAME7 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME7.moos >& /dev/null &
printf "Launching $VNAME8 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME8.moos >& /dev/null &
printf "Launching $VNAME9 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_$VNAME9.moos >& /dev/null &
printf "Done \n"

#-----------------------------------------------------------
#  Part 4: Launch uMAC and kill everything upon exiting uMAC
#-----------------------------------------------------------
uMAC targ_shoreside.moos
printf "Killing all processes ... \n"
kill %1 %2 %3 %4 %5 %6 %7 %8 %9 %10
printf "Done killing processes.   \n"
