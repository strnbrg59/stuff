#!/bin/sh

#######################################################
#Generates ChomboVis documentation in shipping form.
#Requires Python and standard Unix text tools.
#######################################################

PYTHONPATH=../src_py:../lib; export PYTHONPATH
LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH

#
# The purple index-buttons on the left side.
#
python gen_frame1.py > frame1.html

#
# pydoc-generated stuff.
#
index_insert=index.html_insert
modules="visualizable_dataset\
         box_layout_data\
         box_layout\
         box\
         module_maker\
         vtk_line_plot\
         vtk_glyphs \
         annotation_api\
         clip_api\
         cmap_api\
         eb_api\
         grid_api\
         iso_api\
         misc_api\
         particles_api\
         network_api\
         reader_api\
         slice_api\
         stream_api\
         volume_api"

echo '<TABLE border=2>' > $index_insert
for module in $modules
do
    if [ ! $module = 'module_maker' ]; then
        classname=`python upcaseStyle.py $module`
        echo "class ${classname}:" > temp.py
    else
        classname=$module
    fi
    sed -n '/#Cut to here/,/#Cut from here/p' ../src_py/${module}.py >> temp.py
    #if [ $module = 'module_maker' ]; then exit 0; fi
    mv temp.py ${module}.py
    pydoc -w $module
    rm ${module}.py
    sed "s:$HOME::g" ${module}.html > temp.html
    mv temp.html ${module}.html

    echo "<TR><TD><A HREF=\"${module}.html\">$classname</A></TD></TR>" >> $index_insert
done
echo '</TABLE>' >> $index_insert

#
# UsersGuide and other hand-written documentation: conversion from
# htsrc to html.
#

# Process all the htsrc files:
for filename in *.htsrc
do
    base_name=`echo $filename | cut -d. -f 1`
    ./text2html.py max_depth=2 < ${base_name}.htsrc |\
         grep -v "No libserver found" > ${base_name}.html
done

# Do some surgery on API.html:
sed '/Insert classlist here/q' API.html > t.1
sed -n '/Insert classlist here/,//p' API.html > t.2
cat t.1 $index_insert t.2 > t.3
mv t.3 API.html
rm t.1 t.2

