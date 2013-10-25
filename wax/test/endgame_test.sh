#!/bin/sh
TMP=/tmp/wax_endgame.out
CANONICAL=endgame.canonical

../../../bin/wax debug_level=4 max_depth=3 max_moves=5 initial_position_file=..//data/endgame.txt 2>&1 | grep -v Timer > $TMP
cmp $CANONICAL $TMP
if test $? -ne 0 ; then
    echo "!!! Failure (endgame test).  diff $TMP ./$CANONICAL prints:"
    diff $TMP $CANONICAL
else
    echo "  *** Success (endgame test)"
fi
