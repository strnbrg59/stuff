#!/bin/sh

PYTHONPATH=../src_py; export PYTHONPATH
OUTFILE=/tmp/digraph.txt
CANONICAL=../canonicals/digraph.txt
if [ -f $OUTFILE ]; then rm $OUTFILE; touch $OUTFILE; fi

echo "*** cmdline error ***" >> $OUTFILE
python digraph.py foobar >> $OUTFILE 2>&1
echo -n '.'

echo "*** no cmdline args ***" >> $OUTFILE
python digraph.py >> $OUTFILE 2>&1
echo -n '.'

echo "*** show_toplevel=0 ***" >> $OUTFILE
python digraph.py show_toplevel=0 >> $OUTFILE 2>&1
echo -n '.'

echo "*** show_nonchombo=0 ***" >> $OUTFILE
python digraph.py show_nonchombo=0 >> $OUTFILE 2>&1
echo -n '.'

echo "*** show_toplevel=0 show_control=0 ***" >> $OUTFILE
python digraph.py show_toplevel=0 show_control=0 >> $OUTFILE 2>&1
echo -n '.'

echo "*** show_toplevel=0 show_control=0 show_nonchombo=0 ***" >> $OUTFILE
python digraph.py show_toplevel=0 show_control=0 show_nonchombo=0 >> $OUTFILE 2>&1
echo -n '.'

echo "*** single_module=menubar ***" >> $OUTFILE
python digraph.py single_module=menubar >> $OUTFILE 2>&1
echo -n '.'

echo "*** focus=vtk_iso ***" >> $OUTFILE
python digraph.py single_module=menubar focus=1 >> $OUTFILE 2>&1
echo -n '.'

diff $OUTFILE $CANONICAL > /dev/null
if [ $? = 0 ]; then
    echo "*** Passed digraph test ***"
else
    echo "*** Failed digraph test.  diff $CANONICAL $OUTFILE. ***"
fi
