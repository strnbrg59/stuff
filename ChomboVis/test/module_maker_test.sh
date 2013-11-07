#!/bin/sh

#################################################################
# Usage: ./module_maker_test.sh
# Tries out module_maker.py -- on-the-fly Python wrapping.
# Compares the (text) output against a canonical.
# 
# Meant to be run from the share/ChomboVis/test directory.
#
# Normal output looks like this:
# "..........."
# "*** Passed module_maker test."
#################################################################

tmp_dir=/tmp/chombovis_$USER
if [ ! -x $tmp_dir ]; then mkdir $tmp_dir; fi
OUTFILE=module_maker_test.txt
rm -f ${tmp_dir}/$OUTFILE

retval=0

(cd ../examples/module_maker; make > ${tmp_dir}/$OUTFILE)

../../../bin/chombodatalite ../examples/module_maker/runit.py 2>&1 |\
    egrep -v 'ftemplate-depth|share/ChomboVis|emacs_meta_keymap|memberFunctionAudit|GetDataCentering()' \
    >> ${tmp_dir}/${OUTFILE} 2>&1

# Compare text output to canonical.
cmp ${tmp_dir}/$OUTFILE canonicals/$OUTFILE
if [ $? -ne 0 ]; then
    echo "!!! Failed module_maker test.  diff ${tmp_dir}/$OUTFILE canonicals/$OUTFILE"
    retval=`expr $retval + 1`
else
    echo ""
    echo -n "*** Passed module_maker text test."
    echo ""
fi

exit $retval
