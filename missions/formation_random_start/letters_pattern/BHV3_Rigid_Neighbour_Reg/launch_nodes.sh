#!/bin/bash
#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
WARP=1
SWARM_NUM_NODES=22
RANDOM_BOX_WIDTH=200
RANDOM_BOX_HEIGHT=200
JUST_NODES="no"
JUST_SHORE="no"

while :
do
  case $1 in
  -h | --help | -\?)
    printf "%s [SWITCHES] [time_warp] \n" $0
    printf "  --just_build \n\t(shortcuts: -j) \n" 
    printf "  --just_nodes \n\t(shortcuts: -jn) \n" 
    printf "  --just_shore \n\t(shortcuts: -js) \n" 
    printf "  --width=value : width of box within which AUVs are randomly placed, default is $RANDOM_BOX_WIDTH \n\t(shortcuts: -w) \n" 
    printf "  --height=value : height of box within which AUVs are randomly placed, default is $RANDOM_BOX_HEIGHT \n\t(shortcuts: -h) \n" 
    printf "  --time_warp=value : default is $WARP \n\t(shortcuts: -t) \n"  
    printf "  --help \n\t(shortcuts: -h, -?) \n" 
    exit 0
    ;;
  -j | --just_build)
    JUST_MAKE="yes"
    shift
    ;;
  -jn | --just_nodes)
    JUST_NODES="yes"
    shift
    ;;
  -js | --just_shore)
    JUST_SHORE="yes"
    shift
    ;;
  -w=* | --width=*)
    RANDOM_BOX_WIDTH=${1#*=}
    printf "\tNew parameter RANDOM_BOX_WIDTH: %d\n" $RANDOM_BOX_WIDTH
    shift
    ;;
  -h=* | --height=*)
    RANDOM_BOX_HEIGHT=${1#*=}
    printf "\tNew parameter RANDOM_BOX_HEIGHT: %d\n" $RANDOM_BOX_HEIGHT
    shift
    ;;
  -t=* | --time_warp=*)
    WARP=${1#*=}
    printf "\tNew parameter TIME_WARP: %d\n" $WARP
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
printf "\n"

#-----------------------------------------------------------
#  Part 2: Create the .moos and .bhv files. 
#-----------------------------------------------------------
MULTICAST=multicast_1
VTYPE="auv"
SWARM_ORIGIN_X=0
SWARM_ORIGIN_Y=0
SWARM_DEPTH=200
SWARM_PREFIX=NODE_
SWARM_START_PORT=9100
SWARM_START_SHARE_PORT=10100
SHORE_SHARE_PORT="9001"
RANDOM_BOX_WIDTH_SUB=$((RANDOM_BOX_WIDTH / 2))
RANDOM_BOX_HEIGHT_SUB=$((RANDOM_BOX_HEIGHT / 2))

getNodeOffsetsList()
{
  printf "    node_offsets = name="$SWARM_PREFIX$((1))",x="0",y="0"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((2))",x="0",y="100"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((3))",x="0",y="200"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((4))",x="0",y="300"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((5))",x="75",y="230"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((6))",x="150",y="150"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((7))",x="225",y="70"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((8))",x="300",y="0"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((9))",x="300",y="100"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((10))",x="300",y="200"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((11))",x="300",y="300"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((12))",x="500",y="0"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((13))",x="500",y="100"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((14))",x="500",y="200"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((15))",x="500",y="300"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((16))",x="575",y="300"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((17))",x="690",y="290"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((18))",x="800",y="250"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((19))",x="690",y="160"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((20))",x="575",y="150"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((21))",x="687",y="75"\n"
  printf "    node_offsets = name="$SWARM_PREFIX$((22))",x="800",y="0"\n"
}

getNodeOffsetsMetricList()
{
  printf "    node_offsets_metric = x="0",y="0"\n"
  printf "    node_offsets_metric = x="0",y="100"\n"
  printf "    node_offsets_metric = x="0",y="200"\n"
  printf "    node_offsets_metric = x="0",y="300"\n"
  printf "    node_offsets_metric = x="75",y="230"\n"
  printf "    node_offsets_metric = x="150",y="150"\n"
  printf "    node_offsets_metric = x="225",y="70"\n"
  printf "    node_offsets_metric = x="300",y="0"\n"
  printf "    node_offsets_metric = x="300",y="100"\n"
  printf "    node_offsets_metric = x="300",y="200"\n"
  printf "    node_offsets_metric = x="300",y="300"\n"
  printf "    node_offsets_metric = x="500",y="0"\n"
  printf "    node_offsets_metric = x="500",y="100"\n"
  printf "    node_offsets_metric = x="500",y="200"\n"
  printf "    node_offsets_metric = x="500",y="300"\n"
  printf "    node_offsets_metric = x="575",y="300"\n"
  printf "    node_offsets_metric = x="690",y="290"\n"
  printf "    node_offsets_metric = x="800",y="250"\n"
  printf "    node_offsets_metric = x="690",y="160"\n"
  printf "    node_offsets_metric = x="575",y="150"\n"
  printf "    node_offsets_metric = x="687",y="75"\n"
  printf "    node_offsets_metric = x="800",y="0"\n"      
}

getNodeCommunitiesList()
{
  ELCOUNT=0
  printf "    NODE_COMMUNITIES = "
  for ((i = 0; i < $SWARM_NUM_NODES-1; i++))
    do
      printf ""$SWARM_PREFIX$((ELCOUNT+1))","
      ELCOUNT=$((ELCOUNT+1))
  done
  printf "$SWARM_PREFIX$((ELCOUNT+1))" 
}

mkdir -p node_moos
mkdir -p node_bhv
mkdir -p node_logs

COUNT=0
for ((i = 0; i < $SWARM_NUM_NODES; i++))
  do
    NODE_X=$(shuf -i 0-$RANDOM_BOX_WIDTH -n 1)
    NODE_X=$((NODE_X-$RANDOM_BOX_WIDTH_SUB))
    NODE_Y=$(shuf -i 0-$RANDOM_BOX_HEIGHT -n 1)
    NODE_Y=$((NODE_Y-$RANDOM_BOX_HEIGHT_SUB))
    NODE_POS="$NODE_X,$NODE_Y"
    NODE_PORT="$(($SWARM_START_PORT+$((COUNT+1))))"
    NODE_SHARE_PORT="$(($SWARM_START_SHARE_PORT+$((COUNT+1))))"
    NODE_NAME="$SWARM_PREFIX$((COUNT+1))"
    nsplug meta_vehicle.moos ./node_moos/targ_$NODE_NAME.moos -f	\
    VNAME=$NODE_NAME							\
    VPORT=$NODE_PORT							\
    WARP=$WARP								\
    SHARE_LISTEN=$NODE_SHARE_PORT					\
    SHORE_LISTEN=$SHORE_SHARE_PORT					\
    MULTICAST=$MULTICAST						\
    VTYPE=$VTYPE							\
    START_POS=$NODE_POS							\
    MODEM_ID=$((COUNT+1))
    nsplug meta_vehicle.bhv ./node_bhv/targ_$NODE_NAME.bhv -f         	\
    VNAME=$NODE_NAME                                     		\
    NODE_OFFSETS_LIST="$(getNodeOffsetsList)"
    COUNT=$((COUNT+1))
done

echo "Built $SWARM_NUM_NODES node .moos and .bhv files under node_moos and node_bhv."

nsplug meta_shoreside.moos targ_shoreside.moos -f WARP=$WARP \
    VNAME="shoreside"  SHARE_LISTEN=$SHORE_SHARE_PORT        \
    VPORT="9000" MULTICAST=$MULTICAST                        \
    NODE_COMMUNITIES_LIST="$(getNodeCommunitiesList)"	     \
    NODE_OFFSETS_METRIC_LIST="$(getNodeOffsetsMetricList)"

echo "Built shoreside .moos file here."

#-----------------------------------------------------------
#  Part 3: Launch nodes. 
#-----------------------------------------------------------
if [ ${JUST_SHORE} != "yes" ] ; then
  for ((i = 0; i < $SWARM_NUM_NODES; i++))
  do
    NODE_NAME="$SWARM_PREFIX$((i+1))"
    #pAntler ./node_moos/targ_$NODE_NAME.moos >& ./node_output/$NODE_NAME.out &
    pAntler ./node_moos/targ_$NODE_NAME.moos >& /dev/null &
    sleep 0.1
  done
  echo "Launched $SWARM_NUM_NODES nodes."
fi

#-----------------------------------------------------------
#  Part 4: Launch shoreside. 
#-----------------------------------------------------------
if [ ${JUST_NODES} != "yes" ] ; then
  pAntler targ_shoreside.moos >& /dev/null &
  echo "Launched shoreside."
  uMAC targ_shoreside.moos
fi

#-----------------------------------------------------------
#  Part 5: Wait for kill. 
#-----------------------------------------------------------
ANSWER="0"
while [ "${ANSWER}" != "1" -a "${ANSWER}" != "2" ]; do
  echo "(1) Exit script (2) Exit and kill simulation"
  printf "> "
  read ANSWER
done

#-----------------------------------------------------------
#  Part 6: Kill nodes & shoreside. 
#-----------------------------------------------------------
# %1 matches the PID of the first job in the active jobs list, 
# namely the pAntler job launched in Part 4.
if [ "${ANSWER}" = "2" ]; then
  echo "Killing all processes..."
  if [ ${JUST_SHORE} == "yes" ] ; then
    kill %1
  elif [ ${JUST_NODES} == "yes" ] ; then
    for ((i = 1; i <= $SWARM_NUM_NODES; i++))
    do
      kill %$i
    done
  else
    for ((i = 1; i <= $(SWARM_NUM_NODES)+1; i++))
    do
      kill %$i
    done
  fi
  echo "Done killing processes."
fi
