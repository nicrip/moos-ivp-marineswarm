#!/bin/bash 

# M200_IP
#  Emulator running on same machine as vehicle:     localhost
#  Emulator running on different machine:           IP address of that machine (often 192.168.2.1)
#  Actual evan vehicle:                             192.168.5.1
#  Actual felix vehile:                             192.168.6.1
#M200_IP="localhost"
#M200_IP="192.168.254.1"
M200_IP="192.168.X.1"

# SHORE_IP
#  Emulation, shoreside running on same machine as vehicle: localhost
#  Emulation, shoreside running on a different machine:     IP address of that machine (often 192.168.2.1)
#  Actual vehicle:                                          IP address of the shoreside computer
#SHORE_IP="localhost"
#SHORE_IP="192.168.254.1"
SHORE_IP="192.168.1.X"

WARP=1
BAD_ARGS=""
MOOS_FILE=""
BHV_FILE=""

printf "Initiate launch vehicle script\n"

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--xray" -o "${ARGI}" = "-x" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="xray"
        VPORT="9024"
        SHARE_LISTEN="9324"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_xray.moos"
        BHV_FILE="targ_xray.bhv"
	MODEM_ID=24
	POINT_X=0
	POINT_Y=0
        printf "xray vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--whiskey" -o "${ARGI}" = "-w" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="whiskey"
        VPORT="9023"
        SHARE_LISTEN="9323"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_whiskey.moos"
        BHV_FILE="targ_whiskey.bhv"
	MODEM_ID=23
	POINT_X=0
	POINT_Y=0
        printf "whiskey vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--victor" -o "${ARGI}" = "-v" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="victor"
        VPORT="9022"
        SHARE_LISTEN="9322"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_victor.moos"
        BHV_FILE="targ_victor.bhv"
	MODEM_ID=22
	POINT_X=0
	POINT_Y=0
        printf "victor vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--uniform" -o "${ARGI}" = "-u" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="uniform"
        VPORT="9021"
        SHARE_LISTEN="9321"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_uniform.moos"
        BHV_FILE="targ_uniform.bhv"
	MODEM_ID=21
	POINT_X=0
	POINT_Y=0
        printf "uniform vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--tango" -o "${ARGI}" = "-t" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="tango"
        VPORT="9020"
        SHARE_LISTEN="9320"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_tango.moos"
        BHV_FILE="targ_tango.bhv"
	MODEM_ID=20
	POINT_X=0
	POINT_Y=0
        printf "tango vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--sierra" -o "${ARGI}" = "-s" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="sierra"
        VPORT="9019"
        SHARE_LISTEN="9319"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_sierra.moos"
        BHV_FILE="targ_sierra.bhv"
	MODEM_ID=19
	POINT_X=0
	POINT_Y=0
        printf "sierra vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--romeo" -o "${ARGI}" = "-r" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="romeo"
        VPORT="9018"
        SHARE_LISTEN="9318"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_romeo.moos"
        BHV_FILE="targ_romeo.bhv"
	MODEM_ID=18
	POINT_X=0
	POINT_Y=0
        printf "romeo vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--quebec" -o "${ARGI}" = "-q" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="quebec"
        VPORT="9017"
        SHARE_LISTEN="9317"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_quebec.moos"
        BHV_FILE="targ_quebec.bhv"
	MODEM_ID=17
	POINT_X=0
	POINT_Y=0
        printf "quebec vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--papa" -o "${ARGI}" = "-p" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="papa"
        VPORT="9016"
        SHARE_LISTEN="9316"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_papa.moos"
        BHV_FILE="targ_papa.bhv"
	MODEM_ID=16
	POINT_X=0
	POINT_Y=0
        printf "papa vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--oscar" -o "${ARGI}" = "-o" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="oscar"
        VPORT="9015"
        SHARE_LISTEN="9315"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_oscar.moos"
        BHV_FILE="targ_oscar.bhv"
	MODEM_ID=15
	POINT_X=0
	POINT_Y=0
        printf "oscar vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--november" -o "${ARGI}" = "-n" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="november"
        VPORT="9014"
        SHARE_LISTEN="9314"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_november.moos"
        BHV_FILE="targ_november.bhv"
	MODEM_ID=14
	POINT_X=0
	POINT_Y=0
        printf "november vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--mike" -o "${ARGI}" = "-m" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="mike"
        VPORT="9013"
        SHARE_LISTEN="9313"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_mike.moos"
        BHV_FILE="targ_mike.bhv"
	MODEM_ID=13
	POINT_X=0
	POINT_Y=0
        printf "mike vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--lima" -o "${ARGI}" = "-l" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="lima"
        VPORT="9012"
        SHARE_LISTEN="9312"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_lima.moos"
        BHV_FILE="targ_lima.bhv"
	MODEM_ID=12
	POINT_X=0
	POINT_Y=0
        printf "lima vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--kilo" -o "${ARGI}" = "-k" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="kilo"
        VPORT="9011"
        SHARE_LISTEN="9311"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_kilo.moos"
        BHV_FILE="targ_kilo.bhv"
	MODEM_ID=11
	POINT_X=0
	POINT_Y=0
        printf "kilo vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--juliet" -o "${ARGI}" = "-j" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="juliet"
        VPORT="9010"
        SHARE_LISTEN="9310"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_juliet.moos"
        BHV_FILE="targ_juliet.bhv"
	MODEM_ID=10
	POINT_X=80
	POINT_Y=-120
        printf "juliet vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--india" -o "${ARGI}" = "-i" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="india"
        VPORT="9009"
        SHARE_LISTEN="9309"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_india.moos"
        BHV_FILE="targ_india.bhv"
	MODEM_ID=9
	POINT_X=60
	POINT_Y=-120
        printf "india vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--hotel" -o "${ARGI}" = "-h" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="hotel"
        VPORT="9008"
        SHARE_LISTEN="9308"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_hotel.moos"
        BHV_FILE="targ_hotel.bhv"
	MODEM_ID=8
	POINT_X=40
	POINT_Y=-120
        printf "hotel vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--gus" -o "${ARGI}" = "-g" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="gus"
        VPORT="9007"
        SHARE_LISTEN="9307"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_gus.moos"
        BHV_FILE="targ_gus.bhv"
	MODEM_ID=7
	POINT_X=80
	POINT_Y=-100
        printf "GUS vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--felix" -o "${ARGI}" = "-f" ] ; then
        FELIX="yes"
        UNDEFINED_ARG=""
        VNAME="felix"
        VPORT="9006"
        SHARE_LISTEN="9306"
        LOITER_PT="x=50,y=10"
        RETURN_PT="0,-20"
        MOOS_FILE="targ_felix.moos"
        BHV_FILE="targ_felix.bhv"
	MODEM_ID=6
	POINT_X=60
	POINT_Y=-100
        printf "FELIX vehicle selected.\n"
    fi
    if [ "${ARGI}" = "--evan" -o "${ARGI}" = "-e" ] ; then
        EVAN="yes"
        UNDEFINED_ARG=""
        VNAME="evan"
        VPORT="9005"
        SHARE_LISTEN="9305"
        LOITER_PT="x=50,y=0"
        RETURN_PT="30,-15"
        MOOS_FILE="targ_evan.moos"
        BHV_FILE="targ_evan.bhv"
	MODEM_ID=5
	POINT_X=40
	POINT_Y=-100
        printf "EVAN vehicle selected.\n"
    fi
    if [ "${ARGI//[^0-9]/}" -eq "$ARGI" ]; then 
        WARP=$ARGI
        UNDEFINED_ARG=""
        printf "Time warp set.\n"
    fi
    if [ "${UNDEFINED_ARG}" != "" ] ; then
	BAD_ARGS=$UNDEFINED_ARG
    fi
done

#-------------------------------------------------------
#  Part 2: Handle Ill-formed command-line arguments
#-------------------------------------------------------

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files. 
#-------------------------------------------------------

printf "Assembling MOOS file ${MOOS_FILE}\n"

CRUISESPEED="1.5"
SHORE_LISTEN="9300"

nsplug meta_vehicle_fld.moos ${MOOS_FILE} -f \
    VNAME=$VNAME                   \
	VPORT=$VPORT               \
	WARP=$WARP                 \
	SHARE_LISTEN=$SHARE_LISTEN \
	SHORE_LISTEN=$SHORE_LISTEN \
	SHORE_IP=$SHORE_IP         \
        M200_IP=$M200_IP           \
	MODEM_ID=$MODEM_ID	   \
	HOSTIP_FORCE="localhost" 

printf "Assembling BHV file $BHV_FILE\n"
nsplug meta_vehicle.bhv $BHV_FILE -f                  \
    VNAME=$VNAME                                      \
    SPEED=$CRUISESPEED                                \
    ORDER="normal"				      \
    POINT_X=$POINT_X				      \
    POINT_Y=$POINT_Y

if [ ${JUST_BUILD} = "yes" ] ; then
    printf "Files assembled; vehicle not launched; exiting per request.\n"
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

printf "Launching $VNAME MOOS Community \n"
pAntler $MOOS_FILE >& /dev/null &
uMAC $MOOS_FILE

# %1 matches the PID of the first job in the active jobs list, 
# namely the pAntler job launched in Part 4.
if [ "${ANSWER}" = "2" ]; then
    printf "Killing all processes ... \n "
    kill %1 
    printf "Done killing processes.   \n "
fi

