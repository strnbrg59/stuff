#!/bin/sh

bailout='if [ $? -ne 0 ]; then bail; fi'

bail() {
    echo "Inside bail"
    exit $1
}

eject() {
    if [ $? -ne 0 ]; then
        echo $1
        exit 1
    fi
}

cp foo.txt bar.txt
#eval $bailout
eject "@@ cp failed"