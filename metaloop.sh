#!/bin/sh
while [ 1 ]
do \
    read param
    echo $param | ./monolith_bin &
done
