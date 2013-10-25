#!/bin/sh

inet_port=41959
CLIENT=../../../bin/cgi_punclient

#
# Start the server and wait for it to initialize.
#
../../../bin/punserver debug_level=4 punchlines=shaks12.txt inet_port=$inet_port clue_source=inet >& /dev/null &
echo -n "Waiting for server."
while [ 1 ]; do
    $CLIENT inet_port=$inet_port | grep "Server unavailable" > /dev/null
    if [ $? -ne 0 ]; then
        break
    fi
    sleep 1
    echo -n "."
done
echo "ready."

export QUERY_STRING='clue=margot&egrep_iv=&tolerance=5&multispan=on'
$CLIENT debug_level=4 inet_port=$inet_port > /tmp/cgi_canonical.txt 2>&1
export QUERY_STRING='clue=diesel&egrep_iv=this|it is|tis|is all&tolerance=5&multispan=on'
$CLIENT debug_level=4 inet_port=$inet_port >> /tmp/cgi_canonical.txt 2>&1

# One that won't be found in the dictionary:
export QUERY_STRING='clue=wmfxwwpa&egrep_iv=&tolerance=7&multispan=off'
$CLIENT debug_level=4 inet_port=$inet_port >> /tmp/cgi_canonical.txt 2>&1

# One that is in the dictionary but for which we won't find any matches:
export QUERY_STRING='clue=disestablishment&egrep_iv=&tolerance=1&multispan=off'
$CLIENT debug_level=4 inet_port=$inet_port >> /tmp/cgi_canonical.txt 2>&1

diff /tmp/cgi_canonical.txt ./cgi_canonical.txt
if test $? -eq 0; then
    echo "  *** Success"
else
    echo "!!! Failure.  diff /tmp/cgi_canonical.txt ./cgi_canonical.txt prints:"
fi

kill %1
