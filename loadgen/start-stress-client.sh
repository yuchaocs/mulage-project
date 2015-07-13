#!/bin/bash

# $1 ----> scheduler ip
# $2 ----> scheduler port
# $3 ----> distribution file
# $4 ----> query number
# $5 ----> warm up account

scheduler_ip=$1
scheduler_port=$2
distribution_file=$3
query_num=$4
warm_up_account=$5

pinned_core=30

taskset -c $pinned_core java -XX:+UseConcMarkSweepGC -server -Xms1024m -Xmx2048m \
 -jar stressclient.jar $scheduler_ip $scheduler_port $distribution_file $query_num $warm_up_account 2>&1 &
