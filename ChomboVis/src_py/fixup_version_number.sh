#!/bin/sh

# Encode the version number -- found in ../configure.in -- into
# release_number.py.  vtk_data.py looks at that and stores the value as a save:1
# variable.  This way all state files will indicate the number of the ChomboVis
# version that created them.
version=`grep AM_INIT_AUTOMAKE ../configure.in | awk -F"," '{print $2}' | sed 's/)//' | sed 's/ //'`
cat release_number.py | sed "s/\(the_number =\).*$/\1 \'$version\'/" > fixup.temp
cmp fixup.temp release_number.py
if [ $? -ne 0 ]; then
    cp fixup.temp release_number.py
    echo "Inserted new version number into release_number.py:$version"
else
    echo "No need for new version number insertion."
fi
rm fixup.temp
