#!/bin/sh

(cd ../../../libexec/ChomboVis
 if [ -f unittests.sh ]; then
     ./unittests.sh
 else
     ./no_unittests.sh
 fi
)
./module_maker_test.sh
./time_test.sh
./cmdline_test.sh
./api_test.sh
