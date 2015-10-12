#!/bin/sh

# VAR:
  # formations/
  source ../../../config/launch/var.sh
  # formations/blue/
  source ../../config/launch/var.sh
  # formations/blue/isosceles_ping_formation/
  source ../config/launch/var.sh
  # formations/blue/isosceles_ping_formation/square/
  source config/launch/var.sh

# DYNAMIC_PARAM:
  # formations/
  source ../../../config/launch/dynamic_param.sh
  # formations/blue/
  source ../../config/launch/dynamic_param.sh
  # formations/blue/isosceles_ping_formation/
  source ../config/launch/dynamic_param.sh
  # formations/blue/isosceles_ping_formation/square/
  source config/launch/dynamic_param.sh

min_l=-1
max_l=0
for (( i=0 ; i < NUMBER_OF_RECEIVERS ; i++ )); do
  l=$(cat logs/"${VEHICULE_NAME[i]}"_logs.csv | wc -l)
  if ((l<min_l)) || ((min_l=-1)); then
    min_l=$l
  fi
  if ((l>max_l)); then
    max_l=$l
  fi
done
printf "\n"
echo "Minimum nb lines: "$min_l
echo "Maximum nb lines: "$max_l