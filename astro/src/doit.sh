#!/bin/sh

../../bin/astro-static debug_level=4 threshold=50 sample_size=30 transforms_tolerance=3.0 triangle_tolerance=9E-6 triangle_perimeter_min=200 data_suffix=.ppm.gz data_dir=../testdata/

exit 0

cat display1.dat | ~/anag/pyChomboVis/ascii2hdf5.sh outfile=display1.hdf5
cat display2.dat | ~/anag/pyChomboVis/ascii2hdf5.sh outfile=display2.hdf5

echo "Starting chombovis 1..."
~/anag/pyChomboVis/chombovis ignore_rc=1 user_script=show_sample.py display1.hdf5 &
echo "Starting chombovis 2..."
~/anag/pyChomboVis/chombovis ignore_rc=1 user_script=show_sample.py display2.hdf5 
