#!/bin/sh

echo "server test..."
./server_test.sh

# This one needs a running server -- see cgi_test.sh for
# instructions on how to start that server.
echo "cgi test..."
./cgi_test.sh
