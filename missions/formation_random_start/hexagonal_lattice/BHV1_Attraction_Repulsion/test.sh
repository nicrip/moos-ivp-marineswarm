#!/bin/bash
SWARM_ORIGIN_X=0
SWARM_ORIGIN_Y=0
SWARM_NUM_NODES_X=3
SWARM_NUM_NODES_Y=3
SWARM_OFFSET=300
SWARM_OFFSET_ALT_X=$((SWARM_OFFSET / 2))
SWARM_OFFSET_ALT_Y=$(echo "sqrt(($SWARM_OFFSET*$SWARM_OFFSET)-($SWARM_OFFSET_ALT_X*$SWARM_OFFSET_ALT_X))" | bc -l)
echo $SWARM_OFFSET
echo $SWARM_OFFSET_ALT_X
echo $SWARM_OFFSET_ALT_Y

  for ((i = 0; i < $SWARM_NUM_NODES_X; i++))
    do
      for ((j = 0; j < $SWARM_NUM_NODES_Y; j++))
        do
          if [ $((($j+1)%2)) = 0 ]; then
            x_pos=$(echo "$i*$SWARM_OFFSET+$SWARM_ORIGIN_X+$SWARM_OFFSET_ALT_X" | bc -l)
            y_pos=$(echo "$j*$SWARM_OFFSET_ALT_Y+$SWARM_ORIGIN_Y" | bc -l)
          else
            x_pos=$(echo "$i*$SWARM_OFFSET+$SWARM_ORIGIN_X" | bc -l)
            y_pos=$(echo "$j*$SWARM_OFFSET_ALT_Y+$SWARM_ORIGIN_Y" | bc -l)
          fi
      done
  done    
