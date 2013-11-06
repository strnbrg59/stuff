#!/bin/sh

VTKHOME=$HOME/usr/src/ChomboVis-4.16.9-and-everything/usr
export PATH=$VTKHOME/bin:$PATH
export LD_LIBRARY_PATH=$VTKHOME/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$HOME/usr/local/src/rubic/bin

python -i initrubic.py
