#!/bin/sh
./parking n_iterations=100 curb_length=1000 | awk '{print $NF}' \
    | sort | uniq -c | awk '{print $2, $1}' > bargraph.gpd
gnuplot bargraph.gp -
