#! /bin/bash

#find /home/quan/PDA/PDAm/16k -type f -name '*_5.wav' > wav.txt 
while read line
do
	cp $line im-input/
done < im.txt

#find $PWD -type f > input.txt 
#find input -type f > input.txt 
