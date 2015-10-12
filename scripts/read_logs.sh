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

OLDIFS=$IFS
IFS=,
result_duration_estimation=0
min_result_duration_estimation=-1
max_result_duration_estimation=0
mission_time_s=0

for (( i=0 ; i < NUMBER_OF_RECEIVERS ; i++ )); do
  INPUT="logs/"${VEHICULE_NAME[i]}"_logs.csv"
  
  [ ! -f $INPUT ] && { echo "$INPUT file not found"; exit 99; }
  k=0
  while read mission_time MOOStime x y depth speed active_distance drift drift_distance total_distance cons_nom cons_prop cons_com cons_total recording_ratio duration_estimation
  do
    result_duration_estimation=$duration_estimation
    mission_time_s=$mission_time
    k=$((k+1))
  done < $INPUT
  echo "    ...computing $INPUT ($k lines - estim: $result_duration_estimation)"
  
  if [ 1 -eq `echo "${min_result_duration_estimation} == -1" | bc` ] ; then
    min_result_duration_estimation=$result_duration_estimation
  fi
  
  if [ 1 -eq `echo "${result_duration_estimation} < ${min_result_duration_estimation}" | bc` ] ; then
    min_result_duration_estimation=$result_duration_estimation
  fi
  if [ 1 -eq `echo "${result_duration_estimation} > ${max_result_duration_estimation}" | bc` ] ; then
    max_result_duration_estimation=$result_duration_estimation
  fi
done

printf "\n"
echo "Min duration estimation day: $min_result_duration_estimation"
echo "Max duration estimation day: $max_result_duration_estimation"
mission_time_h=$(echo "$mission_time_s/(3600)" | bc -l)
mission_time_d=$(echo "$mission_time_s/(3600*24)" | bc -l)
printf 'Simulation duration: %.2fH (%.2fD)\n' "${mission_time_h/./,}" "${mission_time_d/./,}"
printf "\n"

IFS=$OLDIFS