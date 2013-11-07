#!/bin/sh

#################################################################
# Usage: ./cmdline_test.sh
# Tries out various combinations of command-line options.  Compares
# the text output against a canonical.
# 
# You must run this from its installation directory!
#
# Normal output looks like this:
# "..........."
# "*** Passed cmdline text test."
# "*** Passed cmdline offscreen image-dump test."
#################################################################

tmp_dir=/tmp/chombovis_$USER
if [ ! -x $tmp_dir ]; then mkdir $tmp_dir; fi
OUTFILE=cmdline_test.txt
rm -f ${tmp_dir}/$OUTFILE

retval=0

runit()
{
    echo -n "."
    args="ignore_rc=1 debug_level=4 $1 user_script=cmdline_test.py"
    echo "../../../bin/chombovis $args" >> ${tmp_dir}/$OUTFILE
    ../../../bin/chombovis $args >> ${tmp_dir}/$OUTFILE 2>&1
    if [ $? -ne 0 ]; then echo "Failure.  See ${tmp_dir}/$OUTFILE"; exit $?; fi
    echo "" >> ${tmp_dir}/$OUTFILE
}

OFFSCREEN=cmdline_offscreen.ppm

runit ''
runit "infile=../data/test.3d.double.hdf5 off_screen=1 cmd=c.misc.hardCopy('${tmp_dir}/${OFFSCREEN}')"
runit 'infile=../data/test.3d.double.hdf5 cmd=c.iso.toggleVisibility(1)'
runit 'infile=../data/test.2d.hdf5 test_class=control_iso test_mode=min'
runit 'infile=../data/test.2d.hdf5 test_class=vtk_cmap test_mode=max'
runit 'infile=../data/test.2d.hdf5 texture=0'
runit 'infile=../data/test.3d.double.hdf5 slice_axis=z axis_position=3.14'
runit 'infile=../data/test.3d.float.hdf5 test_class=menubar'
# This one is equivalent to chombodata:
runit 'infile=../data/test.3d.float.hdf5 test_class=reader_api no_vtk=1'
# This one is equivalent to chombobrowser:
runit 'infile=../data/test.3d.float.hdf5 test_class=control_fab_tables no_vtk=1'
runit 'infile=../data/test.3d.float.hdf5 use_render_widget=0'


# Compare text output to canonical.
egrep -v 'emacs_meta_keymap|memberFunctionAudit|GetDataCentering()' ${tmp_dir}/$OUTFILE > ${tmp_dir}/${OUTFILE}.temp
mv ${tmp_dir}/${OUTFILE}.temp ${tmp_dir}/$OUTFILE
cmp ${tmp_dir}/$OUTFILE canonicals/$OUTFILE
if [ $? -ne 0 ]; then
    echo "!!! Failed cmdline test.  diff ${tmp_dir}/$OUTFILE canonicals/$OUTFILE"
    retval=`expr $retval + 1`
else
    echo ""
    echo -n "*** Passed cmdline text test."
    echo ""
fi

# Compare ppm produced in off_screen mode, to canonical (Test case C3.4.6):
cmp ${tmp_dir}/${OFFSCREEN} canonicals/${OFFSCREEN}
if [ $? -ne 0 ]; then
    echo "!!! Failed cmdline offscreen image-dump test.  If you have "
    echo "/usr/X11R6/bin/composite, we will now use it to subtract"
    echo "${tmp_dir}/${OFFSCREEN} from canonicals/${OFFSCREEN}."
    echo "If you see a mostly black image, then you are all right."
    retval=`expr $retval + 1`

    which composite > /dev/null
    have_composite=$?
    which xv > /dev/null
    have_xv=$?
    if [ $have_composite -eq 0 -a $have_xv -eq 0 ]; then
        composite -compose difference canonicals/${OFFSCREEN} ${tmp_dir}/${OFFSCREEN} ${tmp_dir}/diffs.ppm
        xv ${tmp_dir}/diffs.ppm
    fi
else
    echo "*** Passed cmdline offscreen image-dump test."
fi

exit $retval
