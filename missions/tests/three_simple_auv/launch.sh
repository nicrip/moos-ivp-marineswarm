#!/bin/sh

# Number of AUVs in the mission
N=3

# Swarm dimensions
W=2   # Number of AUVs per line
# (N determine the number of AUVs per column)

# Space between AUVs
GAP=300  # Distance (m)

# Name of the file containing the community names
FILE_NAMES_COMMUNITIES="auv_names.txt"

# Shoreside parameters
SHORESIDE_ADDRESS="localhost"
SHORESIDE_PORT="9000"
SHORESIDE_LISTEN="9300"
SHORESIDE_NAME="mothership"

# Vehicles parameters
VEHICLES_ADDRESS="localhost"

# Other
TESTS_ACTIVATION=""
TIME_WARP=5
JUST_MAKE="no"
SHORESIDE_ONLY="no"
VEHICLES_ONLY="no"
NO_CURRENT="no"
ACOUSTIC_RANGE=20050.0
SOUND_SPEED=-1 # not defined
WATER_TEMPERATURE=12.0
WATER_SALINITY=35.0
PING_FREQUENCY=1

# Console display configuration
DISPLAY_WHEN_TESTED="false"
NEVER_DISPLAY="false"
ALWAYS_DISPLAY="true"


#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
    
while :
do
	case $1 in
  -h | --help | -\?)
  	printf "%s [SWITCHES] [time_warp] \n" $0
  	printf "  --just_build \n\t(shortcuts: -j) \n" 
  	printf "  --vehicles_only \n\t(shortcuts: -v) \n" 
  	printf "  --shoreside_only \n\t(shortcuts: -s) \n" 
  	printf "  --nbAUVs=value : number of AUVs in the mission, default is $N \n\t(shortcuts: -n) \n" 
  	printf "  --width=value : number of AUVs per line, default is $W \n\t(shortcuts: -w) \n" 
  	printf "  --gap=value : space between AUVs, default is $GAP \n\t(shortcuts: -g) \n" 
  	printf "  --temperature=value : water temperature used for sound propagation, default is $WATER_TEMPERATURE \n\t(no shortcut) \n" 
  	printf "  --salinity=value : water salinity used for sound propagation, default is $WATER_SALINITY \n\t(no shortcut) \n" 
  	printf "  --range=value : acoustic range between two vehicles, default is $ACOUSTIC_RANGE \n\t(no shortcut) \n" 
  	printf "  --gap=value : space between AUVs, default is $GAP \n\t(shortcuts: -g) \n" 
  	printf "  --time_warp=value : default is $TIME_WARP \n\t(shortcuts: -t) \n" 
  	printf "  --sound_speed=value : sound speed to use (ignore temperature and salinity) \n" 
  	printf "  --ping_frequency=value : frequency between two pings, default is $PING_FREQUENCY \n" 
  	printf "  --test : launching unit tests \n" 
  	printf "  --help \n\t(shortcuts: -h, -?) \n" 
  	exit 0
  	;;
  -j | --just_build)
  	JUST_MAKE="yes"
  	shift
  	;;
  -v | --vehicles_only)
  	VEHICLES_ONLY="yes"
  	printf "Enter shoreside address:  [$SHORESIDE_ADDRESS] "
  	read -r addr
  	if [ -n "$addr" ]; then # if not empty
    SHORESIDE_ADDRESS=$addr
  	fi
  	printf "Enter vehicles address:  [$VEHICLES_ADDRESS] "
  	read -r addr
  	if [ -n "$addr" ]; then # if not empty
    VEHICLES_ADDRESS=$addr
  	fi
  	shift
  	;;
  -s | --shoreside_only)
  	SHORESIDE_ONLY="yes"
  	printf "Enter shoreside address:  [$SHORESIDE_ADDRESS] "
  	read -r addr
  	if [ -n "$addr" ]; then # if not empty
    SHORESIDE_ADDRESS=$addr
  	fi
  	shift
  	;;
  --no_current)
  	NO_CURRENT="yes"
  	shift
  	;;
  -n=* | --nbAUVs=*)
  	N=${1#*=}
  	printf "\tNew parameter N: %d\n" $N
  	shift
  	;;
  -w=* | --width=*)
  	W=${1#*=}
  	printf "\tNew parameter W: %d\n" $W
  	shift
  	;;
  -g=* | --gap=*)
  	GAP=${1#*=}
  	printf "\tNew parameter GAP: %d\n" $GAP
  	shift
  	;;
  -t=* | --time_warp=*)
  	TIME_WARP=${1#*=}
  	printf "\tNew parameter TIME_WARP: %d\n" $TIME_WARP
  	shift
  	;;
  --test)
  	printf "\tLaunching tests...\n"
  	DISPLAY_WHEN_TESTED="true"
  	TESTS_ACTIVATION="-t"
  	shift
  	;;
  --temperature=*)
  	WATER_TEMPERATURE=${1#*=}
  	printf "\tNew parameter WATER_TEMPERATURE: %d\n" $WATER_TEMPERATURE
  	shift
  	;;
  --range=*)
  	ACOUSTIC_RANGE=${1#*=}
  	printf "\tNew parameter ACOUSTIC_RANGE: %d\n" $ACOUSTIC_RANGE
  	shift
  	;;
  --sound_speed=*)
  	SOUND_SPEED=${1#*=}
  	printf "\tNew parameter SOUND_SPEED: %d\n" $SOUND_SPEED
  	shift
  	;;
  --salinity=*)
  	WATER_SALINITY=${1#*=}
  	printf "\tNew parameter WATER_SALINITY: %d\n" $WATER_SALINITY
  	shift
  	;;
  --ping_frequency=*)
  	PING_FREQUENCY=${1#*=}
  	printf "\tNew parameter PING_FREQUENCY: %d\n" $PING_FREQUENCY
  	shift
  	;;
  --) # End of all options
  	shift
  	break
  	;;
  -*)
  	echo "WARN: Unknown option (ignored): $1" >&2
  	shift
  	;;
  *)  # no more options. Stop while loop
  	break
  	;;
	esac
done


#-------------------------------------------------------
#  Part 2: Calculate vehicules parameters
#-------------------------------------------------------

if [ ${SHORESIDE_ONLY} != "yes" ] ; then
	IFS=$'\n' read -d '' -r -a lines < $FILE_NAMES_COMMUNITIES
	x=0
	y=0
	j=0

	for i in "${!lines[@]}"; do 
  if [ -z "${lines[$i]}" ]; then # for empty names
  	break;
  fi
  
  VEHICULE_NAME[i]="${lines[$i]}"
  VEHICULE_PORT[i]="$((9001+i))"
  VEHICULE_SHARE_PORT[i]="$((9301+i))"
  VEHICULE_POS[i]="$((-x*GAP)),$((y*GAP))"
  VEHICULE_POS[i]="0,0"
  ((j++))
  
  ((x++))
  if ((x >= W)) ; then
  	x=0
  	((y++))
  fi
  
  if ((i+1 > N)); then
  	break
  fi
	done

	for (( i=j ; i < N ; i++ )); do # AUV with no name...
  VEHICULE_NAME[i]="AUV_0$((i+1))"
  VEHICULE_PORT[i]="$((9001+i))"
  VEHICULE_SHARE_PORT[i]="$((9301+i))"
  VEHICULE_POS[i]="$((-x*GAP)),$((y*GAP))"
  VEHICULE_POS[i]="0,0"
  
  ((x++))
  if ((x >= W)) ; then
  	x=0
  	((y++))
  fi
	done
fi


#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files. 
#-------------------------------------------------------

# Function searching for nearest neighbors
getMasters() # $1: AUV identifier [0..N-1]
{
	id=$1
	
	# "W-N" case:
  printf " BHV_PIF_WN_CONTACT1="
	if [[ $(( (id + 1) % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id + 1]}
  fi
  printf " BHV_PIF_WN_CONTACT2="
	if (( (id + W) < N )) ; then
    printf ${VEHICULE_NAME[id + W]}
  fi
	
	# "N-E" case:
  printf " BHV_PIF_NE_CONTACT1="
	if (( (id + W) < N )) ; then
    printf ${VEHICULE_NAME[id + W]}
  fi
  printf " BHV_PIF_NE_CONTACT2="
	if [[ $(( id % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id - 1]}
  fi
	
	# "E-S" case:
  printf " BHV_PIF_ES_CONTACT1="
	if [[ $(( id % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id - 1]}
  fi
  printf " BHV_PIF_ES_CONTACT2="
	if (( (id - W) >= 0 )) ; then
    printf ${VEHICULE_NAME[id - W]}
  fi
	
	# "S-W" case:
  printf " BHV_PIF_SW_CONTACT1="
	if (( (id - W) >= 0 )) ; then
    printf ${VEHICULE_NAME[id - W]}
  fi
  printf " BHV_PIF_SW_CONTACT2="
	if [[ $(( (id + 1) % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id + 1]}
  fi
	
	# "N-S" case:
  printf " BHV_PIF_NS_CONTACT1="
	if (( (id - W) >= 0 )) ; then
    printf ${VEHICULE_NAME[id - W]}
  fi
  printf " BHV_PIF_NS_CONTACT2="
	if (( (id + W) < N )) ; then
    printf ${VEHICULE_NAME[id + W]}
  fi
	
	# "W-E" case:
  printf " BHV_PIF_WE_CONTACT1="
	if [[ $(( id % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id - 1]}
  fi
  printf " BHV_PIF_WE_CONTACT2="
	if [[ $(( (id + 1) % W )) != 0 ]] ; then
    printf ${VEHICULE_NAME[id + 1]}
  fi
}

if [ ${VEHICLES_ONLY} != "yes" ] ; then
	# .moos
	nsplug ./meta_shoreside.moos targ_$SHORESIDE_NAME.moos -f \
  WARP=$TIME_WARP \
  SNAME=$SHORESIDE_NAME \
  SHARE_LISTEN=$SHORESIDE_LISTEN \
  SPORT=$SHORESIDE_PORT \
  SHORESIDE_ADDRESS=$SHORESIDE_ADDRESS \
  ACOUSTIC_RANGE=$ACOUSTIC_RANGE \
  WATER_TEMPERATURE=$WATER_TEMPERATURE \
  WATER_SALINITY=$WATER_SALINITY \
  PING_FREQUENCY=$PING_FREQUENCY \
  ALWAYS_DISPLAY=$ALWAYS_DISPLAY \
  NEVER_DISPLAY=$NEVER_DISPLAY \
  DISPLAY_WHEN_TESTED=$DISPLAY_WHEN_TESTED \
  TESTS_ACTIVATION=$TESTS_ACTIVATION \
  SOUND_SPEED=$SOUND_SPEED
fi

if [ ${SHORESIDE_ONLY} != "yes" ] ; then
	for (( i=0 ; i<N ; i++ )); do 
  # .moos
  nsplug ./meta_vehicle.moos targ_"${VEHICULE_NAME[i]}".moos -f \
  	WARP=$TIME_WARP \
  	VNAME="${VEHICULE_NAME[i]}" \
  	START_POS="${VEHICULE_POS[i]}" \
  	VPORT="${VEHICULE_PORT[i]}" \
  	SHARE_LISTEN="${VEHICULE_SHARE_PORT[i]}" \
  	VTYPE=UUV \
  	SHORE_LISTEN=$SHORESIDE_LISTEN \
  	SHORESIDE_ADDRESS=$SHORESIDE_ADDRESS \
  	VEHICLES_ADDRESS=$VEHICLES_ADDRESS \
  	ACOUSTIC_RANGE=$ACOUSTIC_RANGE \
  	WATER_TEMPERATURE=$WATER_TEMPERATURE \
  	WATER_SALINITY=$WATER_SALINITY \
  	PING_FREQUENCY=$PING_FREQUENCY \
  	ALWAYS_DISPLAY=$ALWAYS_DISPLAY \
  	NEVER_DISPLAY=$NEVER_DISPLAY \
  	DISPLAY_WHEN_TESTED=$DISPLAY_WHEN_TESTED \
  	TESTS_ACTIVATION=$TESTS_ACTIVATION \
  	SOUND_SPEED=$SOUND_SPEED
  
  # .bhv
  nsplug ./meta_vehicle.bhv targ_"${VEHICULE_NAME[i]}".bhv -f \
  	VNAME="${VEHICULE_NAME[i]}" \
  	START_POS="${VEHICULE_POS[i]}" \
  	$(getMasters $i)
	done
fi

if [ ${JUST_MAKE} = "yes" ] ; then
	exit 0
fi

	
#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

NpAntler=0
printf "\n"

if [ ${VEHICLES_ONLY} != "yes" ] ; then
	printf "Launching $SHORESIDE_NAME MOOS Community (WARP=%s) \n" $TIME_WARP
	pAntler targ_$SHORESIDE_NAME.moos >& /dev/null &
	((NpAntler++))
fi

if [ ${SHORESIDE_ONLY} != "yes" ] ; then
	for (( i=0 ; i<N ; i++ )); do 
  sleep 0.25
  printf "Launching ${VEHICULE_NAME[i]} MOOS Community (WARP=%s) \n" $TIME_WARP
  pAntler targ_"${VEHICULE_NAME[i]}".moos >& /dev/null &
  ((NpAntler++))
	done
fi

printf "Done \n"


#-------------------------------------------------------
#  Part 5: Exiting and/or killing the simulation
#-------------------------------------------------------

ANSWER="0"
while [ "${ANSWER}" != "2" -a "${ANSWER}" != "q" ]; do
    printf "Hit (q) to Exit and Kill Simulation \n"
    printf "> "
    read ANSWER
done

# %1, %2, %3, ... matches the PID of the first N jobs in the active
# jobs list, namely the N pAntler jobs launched in Part 4
if [ "${ANSWER}" = "q"  -o "${ANSWER}" = "2" ]; then
    printf "Killing all processes ... \n"
    for (( i=1 ; i < NpAntler+1 ; i++ )); do 
  kill %$i
	done
  rm targ_*
  mykill &>/dev/null
  printf "Done killing processes.   \n"
fi