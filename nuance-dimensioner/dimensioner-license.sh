#!/bin/sh

echo Content-type: text/html
echo
echo
############## security stuff ############
# turn off file globbing
set -f
# reject bogus (probably dangerous) characters
echo $QUERY_STRING | grep [\;\*\>\<\`\|] > /dev/null
if [ $? -eq 0 ]; then
    echo "Illegal character(s) found in QUERY_STRING.  Exiting now."
    env | mail -s "Evil QUERY_STRING" strnbrg@nuance.com
    exit 1
fi
########## end of security stuff #########

/usr/bin/java -cp Dimensioner.jar PKC encryptCGI $QUERY_STRING

if [ $? -ne 0 ]; then
    echo "<CENTER><FONT SIZE=12><FONT COLOR=\"#FF0000\"> Error </FONT></FONT></CENTER>"
fi
