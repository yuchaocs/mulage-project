#!/bin/bash

asrdir=asr-mulage
immdir=im-mulage
qadir=qa-mulage

service_ip=clarity28.eecs.umich.edu

asr_service_port=9070
imm_service_port=9080
qa_service_port=9090

scheduler_ip=141.212.107.226
scheduler_port=8888


echo "=========================================="
cd $asrdir
echo "starting the speech recognition service..."
cd - > /dev/null
sleep(5)

echo "=========================================="
cd $immdir
echo "starting the image matching service..."
cd - > /dev/null
sleep(7)

echo "=========================================="
cd $qadir
echo "starting the question answer service..."
./start-qa-service.sh $service_ip $qa_service_port $scheduler_ip $scheduler_port
sleep(10)

echo "all done"
