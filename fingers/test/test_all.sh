#!/bin/sh                                                                        

OUTFILE=/tmp/fingers.out
../../../bin/fingers > $OUTFILE 2>&1
cmp canonical.out $OUTFILE
if [ $? -ne 0 ]; then
    echo "!!! Failure.  diff canonical.out $OUTFILE prints:"
    diff canonical.out $OUTFILE
else
    echo "  *** Success"
fi

