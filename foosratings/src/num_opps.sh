#!/bin/sh

players=`cat pingpong.txt | awk '{printf "%s\n%s\n", $1, $3}' | sort | uniq`
for p in $players; do
    opps=`cat pingpong.txt | egrep "^$p | $p " | awk '{printf "%s\n%s\n", $1, $3}' | sort | grep -v $p | uniq | wc -l`
    echo $p ":" $opps
done


