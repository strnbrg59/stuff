#!/bin/sh

for g in *.gz; do
    cp $g ${g}.bak
    gzip -df $g
    mv ${g}.bak $g
    u=`echo $g | sed 's/\.gz//'`
done
