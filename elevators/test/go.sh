#!/bin/sh
../src/elevators > /tmp/temp.out
diff /tmp/temp.out canonical.out
