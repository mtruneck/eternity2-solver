#!/bin/bash

WORKERS=$(lscpu | grep '^CPU(s):' | awk '{ print $2 }')
for i in $(seq 1 $WORKERS); do
    /home/ubuntu/runner.sh &>> /home/ubuntu/results$i &
done

wait

