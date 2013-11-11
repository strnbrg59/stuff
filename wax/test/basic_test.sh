#!/bin/sh
TMP=/tmp/wax_basic.out
CANONICAL=basic.canonical

../../../bin/wax max_depth=3 max_moves=4 2>&1 | egrep -v Timer > $TMP 2>&1
cmp $CANONICAL $TMP
if test $? -ne 0 ; then
    echo "!!! Failure (basic test).  diff $TMP ./$CANONICAL prints:"
    diff $TMP $CANONICAL
else
    echo "  *** Success (basic test)"
fi
