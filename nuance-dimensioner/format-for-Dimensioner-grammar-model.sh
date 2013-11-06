#!/bin/sh

if [ $# -eq 0 ]; then
echo  =========================================================
echo  Take grammar information extracted from the output of batchrec-analyze.tcl,
echo  in a format like this:
echo 
echo               grammar              LU       dur      requests/call
echo            .FAskIfKnowsFlightNumber 2.571000 1.487000 117
echo                .FAskForFlightNumber 1.541000 2.390000 83
echo 
echo  And convert it to the format that the Java dimensioner can load, whose format
echo  has to be like this:
echo 
echo 1.0!false!Narrow yes/no!0.92!1.01!http://intranet/provisioning/yes-no-narrow.html
echo 2.71!false!8-digit strings!1.11!4.93!http://intranet/provisioning/8-digits.html
echo 3.14!false!Dates!2.18!1.88!http://intranet/provisioning/date.html
echo 2.0!true!Time of day!1.91!1.85!http://intranet/provisioning/none_yet.html
echo 
echo
echo "Usage: $0 infile"
echo  =========================================================
exit 1
fi

infile=$1
while read line; do

    set `echo $line`
    echo "$4!false!$1!$2!$3!foobar.html"
done < $infile
