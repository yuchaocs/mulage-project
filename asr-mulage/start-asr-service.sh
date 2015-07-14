#!/usr/bin/env bash

# start the QA server
# start from top directory
# cd ../question-answer ;

# $1 ----> service ip
# $2 ----> service port
# $3 ----> scheduler ip
# $4 ----> scheduler port
# $5 ----> instance number

service_ip=$1
service_port=$2
scheduler_ip=$3
scheduler_port=$4
num_instance=$5
queuing_policy=$6
freq=$7

pinned_core=1
i=0

while [ $i -lt $num_instance ]
do
	taskset -c $pinned_core ./asrservice $service_ip $service_port $scheduler_ip $scheduler_port $queuing_policy $pinned_core $freq > start"$i".log 2>&1 &
	i=`expr $i + 1`
	service_port=`expr $service_port + 1`
	pinned_core=`expr $pinned_core + 1`
done
