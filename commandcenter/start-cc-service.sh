#!/bin/bash

# $1 ----> scheduler port
# $2 ----> adjust interval
# $3 ----> withdraw interval
# $4 ----> warm up account
# $5 ----> adjust threshold
# $6 ----> tail percentile
# $7 ----> global power budget
# $8 ----> execution mode

scheduler_port=$1
adjust_interval=$2
withdraw_interval=$3
warm_up_account=$4
adjust_threshold=$5
tail_percentile=$6
gobal_power_budget=$7
execution_mode=$8
decision_policy=$9
withdraw_instance=$10

pinned_core=31

taskset -c $pinned_core java -XX:+UseConcMarkSweepGC -server -Xms1024m -Xmx2048m \
 -jar commandcenter.jar $scheduler_port $adjust_interval $withdraw_interval $warm_up_account $adjust_threshold $tail_percentile $gobal_power_budget $execution_mode $decision_policy $withdraw_instance 2>&1 &
