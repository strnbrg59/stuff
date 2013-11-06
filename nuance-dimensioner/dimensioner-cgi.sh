#!/bin/sh

echo Content-type: text/html
echo

java -cp Dimens.jar CGIMain $CONTENT_LENGTH
