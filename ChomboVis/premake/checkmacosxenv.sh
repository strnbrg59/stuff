#!/bin/sh

#
# Called from Makefile, and $1 is $host (as determined by
# AC_CANONICAL_HOST).
#

echo $1 | grep darwin > /dev/null
if ! test $? -eq 0 ; then
    exit 0
fi

if test -z $MACOSX_DEPLOYMENT_TARGET; then
    echo "==========================================================="
    echo "Error: you need MACOSX_DEPLOYMENT_TARGET defined (to 10.2, "
    echo "10.3, or whatever) in your environment.                    "
    echo "==========================================================="
    exit 1
fi
