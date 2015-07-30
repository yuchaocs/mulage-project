#!/bin/bash

ccdir=commandcenter
asrdir=asr-mulage
immdir=im-mulage
qadir=qa-mulage
scdir=loadgen

#==================================
# command center configuration
#==================================

scheduler_ip="clarity28.eecs.umich.edu"
scheduler_port=8888
adjust_interval=50
withdraw_interval=150
warm_up_account=20
adjust_threshold=1000
tail_percentile=99
global_power_budget=13.56
#execution_mode="recycle"
execution_mode="vanilla"
#decision_policy="adaptive"
decision_policy="instance"
#decision_policy="frequency"
#withdraw_instance="withdraw"
withdraw_instance="no-withdraw"
latency_result_file="query_latency.csv"
delay_result_file="expected_delay.csv"
frequency_result_file="frequency.csv"

#==================================
# service configuration
#==================================

service_ip="clarity28.eecs.umich.edu"
asr_service_port=9074
imm_service_port=9084
qa_service_port=9094

asr_num_client=3
imm_num_client=3
qa_num_client=5
queuing_policy="fifo"
#queuing_policy="priority"
running_frequency=1.8

# asr_instance_core=4
# imm_instance_core=5
# qa_instance_core=11

#==================================
# stress client configuration
#==================================

query_num=1000
distribution_file="poisson_sample_2.0_1000.csv"
#distribution_file="poisson_sample_.6_1000.csv"
#distribution_file="poisson_sample_1.5_1000.csv"
#distribution_file="poisson_sample_1.2_1000.csv"
#distribution_file="poisson_sample_.6_1000.csv"
#load_type="burst"
load_type="poisson"
#load_type="exponential"
operation_type="load"
#operation_type="sample"

# core assignment
# ASR: core 1 to 5
# IMM: core 6 to 10
# QA: core 11 to 15

function perform_experiment() {
cd $ccdir

echo "starting the command center..."
./start-cc-service.sh $scheduler_port $adjust_interval $withdraw_interval $warm_up_account $adjust_threshold $tail_percentile $global_power_budget $execution_mode $decision_policy $withdraw_instance
cd - > /dev/null
sleep 10

echo "=========================================="
cd $qadir
echo "starting the question answer service..."
./start-qa-service.sh $service_ip $qa_service_port $scheduler_ip $scheduler_port $qa_num_client $queuing_policy $running_frequency
cd - > /dev/null
sleep 10

echo "=========================================="
cd $immdir
echo "starting the image matching service..."
./start-im-service.sh $service_ip $imm_service_port $scheduler_ip $scheduler_port $imm_num_client $queuing_policy $running_frequency
cd - > /dev/null
sleep 10

echo "=========================================="
cd $asrdir
echo "starting the speech recognition service..."
./start-asr-service.sh $service_ip $asr_service_port $scheduler_ip $scheduler_port $asr_num_client $queuing_policy $running_frequency
cd - > /dev/null
sleep 10

sleep 120

echo "command center and all services are running..."
echo "=========================================="
cd $scdir
echo "starting the stress client to submit queries..."
./start-stress-client.sh $scheduler_ip $scheduler_port $distribution_file $query_num $warm_up_account $load_type $operation_type
cd - > /dev/null

echo "waiting for all queries to finish..."
cd $ccdir
finished_queries=`cat $latency_result_file |wc -l`
while [ $finished_queries -lt `expr $query_num + 1` ]
do
	echo "$finished_queries queries have been finished..."
	sleep 20
	finished_queries=`cat $latency_result_file |wc -l`
done

echo "all queries have been finished..."
echo "stopping asr services..."
kill -9 `pidof asrservice`
echo "stopping imm services..."
kill -9 `pidof imservice`
echo "stopping command center and qa services...."
kill -9 `pidof java`
}

#input_list=("high" "medium" "low")
input_list=(".8" ".9" "1.0" "1.1" "1.2" "1.3" "1.4" "1.5" "1.6" "1.7" "1.8" "1.9" "2.0")
policy_list=("naive" "freq" "instance")

for input in ${input_list[@]}
do
	echo "=========================================="
	echo "start the ${input} load experiment"
	distribution_file="poisson_sample_${input}_1000.csv"
	
	for policy in ${policy_list[@]}
	do
	       	echo "=========================================="
        	echo "start the ${policy} boosting experiment"
        	if [ "${policy}" == "naive" ]
        	then
                	execution_mode="vanilla"
        	elif [ "${policy}" == "freq" ]
        	then
                	execution_mode="recycle"
                	decision_policy="frequency"
        	elif [ "${policy}" == "instance" ]
        	then
                	execution_mode="recycle"
                	decision_policy="instance"
        	fi
		
		echo "adjusting the initial service frequency..."
		echo "All cores--->1.8GHz"
	
		for((i=0;i<32;i++))
		do
        		sudo cpufreq-set -c $i -f 1800000
		done

		perform_experiment

		echo "copy experiment results to experiments/swap/${policy}/${input}"

		mkdir ../experiments/swap/${policy}/${input}
		cp $latency_result_file ../experiments/swap/${policy}/${input}
		cp $delay_result_file ../experiments/swap/${policy}/${input}
		cp $frequency_result_file ../experiments/swap/${policy}/${input}
		cp log/commandcenter.log ../experiments/swap/${policy}/${input}
		
		cd - > /dev/null

		sleep 10
	done
done
