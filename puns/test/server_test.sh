#!/bin/sh

../../../bin/punserver punchlines=shaks12.txt clue_source=./clues.txt > /tmp/server_canonical.txt 2>&1
cmp server_canonical.txt /tmp/server_canonical.txt
if test $? -ne 0 ; then
    echo "!!! Failure.  diff /tmp/server_canonical.txt ./server_canonical.txt prints:"
    diff /tmp/server_canonical.txt ./server_canonical.txt
else
    echo "  *** Success"
fi
