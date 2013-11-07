#!/bin/sh

if [ $# -ne 2 ]; then
    echo "**********************************************************************"
    echo "Usage: $0 something.hdf5 slave_pid"
    echo ""
    echo "Loads new hdf5 file into a ChomboVis process running in slave mode."
    echo "Before running this script, start a ChomboVis process in slave mode,"
    echo "and read its pid from a FIFO it creates:"
    echo "$ chombovis cmd='c.network.beSlave(1)' &"
    echo "$ slave_pid=\`cat /tmp/chombovis_$USER/fifo\`"
    echo "...and then run this script, e.g."
    echo "$0 something.hdf5 \$slave_pid"
    echo "**********************************************************************"
    exit 1
fi

cmd="c.loadHDF5('$1')"
echo "cmd=$cmd"
echo "c.reader.loadHDF5('$1')" > /tmp/chombovis_$USER/fifo
kill -SIGUSR1 $2
