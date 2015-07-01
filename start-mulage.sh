#!/bin/bash

asrdir=asr-mulage
immdir=im-mulage
qadir=qa-mulage

service_ip=clarity28.eecs.umich.edu

asr_service_port=9070
imm_service_port=9080
qa_service_port=9090

asr_num_client=1
imm_num_client=1
qa_num_client=4



scheduler_ip=141.212.107.226
scheduler_port=8888

echo "=========================================="
cd $qadir
echo "starting the question answer service..."
./start-qa-service.sh $service_ip $qa_service_port $scheduler_ip $scheduler_port $qa_num_client
cd - > /dev/null
sleep 5

echo "=========================================="
cd $immdir
echo "starting the image matching service..."
./start-im-service.sh $service_ip $imm_service_port $scheduler_ip $scheduler_port $imm_num_client
cd - > /dev/null
sleep 5

echo "=========================================="
cd $asrdir
echo "starting the speech recognition service..."
./start-asr-service.sh $service_ip $asr_service_port $scheduler_ip $scheduler_port $asr_num_client
sleep 5

echo "all done"
