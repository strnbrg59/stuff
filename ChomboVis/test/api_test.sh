#!/bin/sh

#################################################################
# Usage: ./api_test.sh [2dCell|3dCell|2dNode|3dParticles|nodata|nansinfs]
# Args 2dCell, 3dCell etc specify tests to run.  If you don't mention any, we
# run through all the tests.  Each test corresponds to a different hdf5 file
# we store in the data directory.  The file names can be found in
# examples/api_test.py.
#
# You must run this from its installation directory!
#
# Gives the API a good workout.
# 
# Normal output looks like this:
# -------- cut here --------
# *** Passed ebclip2d image-dump test.
# *** Passed ebclip2d text test.
# *** Passed ebclip3d image-dump test.
# *** Passed ebclip3d text test.
# *** Passed Reslice image-dump test.
# *** Passed Reslice text test.
# *** Passed 2dCell image-dump test.
# *** Passed 2dCell text test.
# *** Passed 2dNode image-dump test.
# *** Passed 2dNode text test.
# *** Passed 3dParticles image-dump test.
# *** Passed 3dParticles text test.
# *** Passed 3dCell image-dump test.
# *** Passed 3dCell text test.
# *** Passed nodata image-dump test.
# *** Passed nodata text test.
# *** Passed nansinfs image-dump test.
# *** Passed nansinfs text test.
# -------- cut here --------
#################################################################

OUTFILE=api_test
retval=0

tmp_dir=/tmp/chombovis_$USER
if [ ! -x $tmp_dir ]; then mkdir $tmp_dir; fi

#
# Deal with command-line control over which test to run.
#
if [ $# -eq 0 ]; then
    testname_range="ebclip2d ebclip3d nansinfs Reslice 2dCell 2dNode 3dParticles 3dCell nodata"
    echo "testname_range=$testname_range"
else
    testname_range=$*
fi

for testname in $testname_range; do
    export CHOMBO_TESTNAME=$testname
    echo $testname | grep Reslice > /dev/null
    if [ $? -eq 0 ]; then
        RESLICE_CMD="slice_axis=y axis_position=2.9"
    else
        RESLICE_CMD=
    fi
    ../../../bin/chombovis ignore_rc=1 $RESLICE_CMD user_script=api_test.py 2>&1 | grep -v ^Loading > ${tmp_dir}/$OUTFILE${testname}.txt
    # Excluding "Loading" because it's followed by a number of periods that
    # depend on how long it takes to load the hdf5 file.

    # Compare ppm output to canonical.
    cmp ${tmp_dir}/$OUTFILE${testname}.ppm canonicals/$OUTFILE${testname}.ppm
    if [ $? -ne 0 ]; then
        echo "!!! Failed ${testname} image-dump test.  If you have "
        echo "/usr/X11R6/bin/composite, we will now use it to subtract"
        echo "${tmp_dir}/$OUTFILE${testname}.ppm from canonicals/$OUTFILE${testname}.ppm."
        echo "If you see a mostly black image, then you are all right."
        retval=`expr $retval + 1`

        which composite > /dev/null
        have_composite=$?
        which xv > /dev/null
        have_xv=$?
        if [ $have_composite -eq 0 -a $have_xv -eq 0 ]; then
            composite -compose difference canonicals/$OUTFILE${testname}.ppm ${tmp_dir}/$OUTFILE${testname}.ppm ${tmp_dir}/diffs.ppm
            xv ${tmp_dir}/diffs.ppm
        fi
    else
        echo "*** Passed ${testname} image-dump test."
    fi

    # Compare text output to canonical, modulo some known randomness.
    grep -v 'memberFunctionAudit' ${tmp_dir}/$OUTFILE${testname}.txt | egrep -v 'table|constraint does not|GetDataCentering()|Trying to delete object with|Missing input|vtkGlyph3D.cxx|emacs_meta_keymap' > ${tmp_dir}/$OUTFILE${testname}.temp
    mv ${tmp_dir}/$OUTFILE${testname}.temp ${tmp_dir}/$OUTFILE${testname}.txt
    diff  ${tmp_dir}/$OUTFILE${testname}.txt canonicals/$OUTFILE${testname}.txt > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "!!! Failed ${testname} text test."
        echo "diff ${tmp_dir}/$OUTFILE${testname}.txt canonicals/$OUTFILE${testname}.txt prints..."
        diff ${tmp_dir}/$OUTFILE${testname}.txt canonicals/$OUTFILE${testname}.txt
        retval=`expr $retval + 1`
    else
        echo "*** Passed ${testname} text test."
    fi
done

exit $retval
