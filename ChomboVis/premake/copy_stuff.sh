#!/bin/sh

# Need top_srcdir and top_builddir as absolute paths, or outofsource
# build fails.
old_pwd=`pwd`
cd $1
top_srcdir=`pwd`
cd $old_pwd
cd $2
top_builddir=`pwd`
cd $old_pwd
echo "top_srcdir=$top_srcdir"
echo "top_builddir=$top_builddir"

# Exit (i.e. don't copy files) if we're building in-source.
if test -f ${top_builddir}/configure.in; then
    exit 0
fi

cp -f ${top_srcdir}/user.make ${top_builddir}
#egrep -v '^ *#|^$' ${top_builddir}/user.make
cp -f ${top_srcdir}/doc/*.py ${top_builddir}/doc
cp -f ${top_srcdir}/doc/*.htsrc ${top_builddir}/doc
cp -f ${top_srcdir}/src_py/*.py ${top_builddir}/src_py
cp -f ${top_srcdir}/src_py/fixup_version_number.sh ${top_builddir}/src_py
cp -f ${top_srcdir}/data/*.gz ${top_builddir}/data
cp -f ${top_srcdir}/utils/boost.tar.gz ${top_builddir}/utils
(cd ${top_srcdir}/utils; tar xvfz boost.tar.gz)
cp -f ${top_srcdir}/data/gunzip.sh ${top_builddir}/data
cp -f ${top_srcdir}/test/canonicals/*.gz ${top_builddir}/test/canonicals
cp -f ${top_srcdir}/test/canonicals/*.txt ${top_builddir}/test/canonicals
cp -f ${top_srcdir}/examples/*.{sh,py} ${top_builddir}/examples
mkdir ${top_builddir}/examples/module_maker
cp -f ${top_srcdir}/examples/module_maker/{*.{h,cpp,py},Readme} ${top_builddir}/examples/module_maker
