#!/usr/bin/env bash

# start the QA server
# start from top directory
# cd ../question-answer ;

export CLASSPATH=bin:lib/ml/maxent.jar:lib/ml/minorthird.jar:lib/nlp/jwnl.jar:lib/nlp/lingpipe.jar:lib/nlp/opennlp-tools.jar:lib/nlp/plingstemmer.jar:lib/nlp/snowball.jar:lib/nlp/stanford-ner.jar:lib/nlp/stanford-parser.jar:lib/nlp/stanford-postagger.jar:lib/qa/javelin.jar:lib/search/bing-search-java-sdk.jar:lib/search/googleapi.jar:lib/search/indri.jar:lib/search/yahoosearch.jar:lib/util/commons-logging.jar:lib/util/gson.jar:lib/util/htmlparser.jar:lib/util/log4j.jar:lib/util/trove.jar:lib/util/servlet-api.jar:lib/util/jetty-all.jar:lib/util/commons-codec-1.9.jar:lib/thrift/commons-codec-1.6.jar:lib/thrift/httpclient-4.2.5.jar:lib/thrift/httpcore-4.2.4.jar:lib/thrift/junit-4.4.jar:lib/thrift/libthrift-0.9.2.jar:lib/thrift/slf4j-api-1.5.8.jar:lib/thrift/slf4j-log4j12-1.5.8.jar:lib/util/opencsv-3.4.jar

export INDRI_INDEX=`pwd`/wiki_indri_index/
export THREADS=8

# $1 ----> service ip
# $2 ----> service port
# $3 ----> scheduler ip
# $4 ----> scheduler port
# $5 ----> instance number
# $6 ----> queuing policy

service_ip=$1
service_port=$2
scheduer_ip=$3
scheduler_port=$4
num_instance=$5
queuing_policy=$6

pinned_core=11
i=0

while [ $i -lt $num_instance ]
do
	taskset -c $pinned_core java -XX:+UseConcMarkSweepGC -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m \
  info.ephyra.OpenEphyraService $service_ip $service_port $scheduer_ip $scheduler_port $queuing_policy > start"$i".log 2>&1 &
	i=`expr $i + 1`
	service_port=`expr $service_port + 1`
	pinned_core=`expr $pinned_core + 1`
done
#java -XX:+UseConcMarkSweepGC -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m \
#  info.ephyra.OpenEphyraService clarity28.eecs.umich.edu 9192 141.212.107.226 8888 > start2.log 2>&1 &
#java -XX:+UseConcMarkSweepGC -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m \
#  info.ephyra.OpenEphyraService clarity28.eecs.umich.edu 9193 141.212.107.226 8888 > start3.log 2>&1 &
#java -XX:+UseConcMarkSweepGC -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m \
#  info.ephyra.OpenEphyraService clarity28.eecs.umich.edu 9194 141.212.107.226 8888 > start4.log 2>&1 &
#java -Djava.library.path=lib/search/ -server -Xms1024m -Xmx2048m \
#  info.ephyra.OpenEphyraService
