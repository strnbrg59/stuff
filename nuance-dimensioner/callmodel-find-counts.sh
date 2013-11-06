#!/bin/sh

echo "Content-type: text/plain"
echo ""

#echo "Inside callmodel-find-counts.sh"

java -classpath /usr/java/lib/classes.zip:Dimensioner.jar CallModel cgi $CONTENT_LENGTH


