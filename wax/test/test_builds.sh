#!/bin/sh

#
# Run this on a fresh checkout.  It'll build in-source, from a "make dist",
# and out-of-source against the dist.
#

#
# Usage: $0 [1|0]
# Set the optional arg to 1 if you only want the outofsource build.
#
if test $# -eq 1 ; then
  outofsource_only=1
else
  outofsource_only=0
fi

cd `dirname $0`/..

VERSION=`grep AM_INIT_AUTOMAKE configure.in | cut -d, -f2 | sed 's/)//' | awk '{print $1}'`

#
# In-source: don't bother, as we test it all the time.
#
./bootstrap
ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

install_prefix=$HOME/usr/local

./configure --prefix=$install_prefix
make uninstall
ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

if test $outofsource_only -eq 0; then
  make install
  ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

  if test -d ${install_prefix}/share/wax/test; then
    echo "@@@ Running in-source test..."
    ( cd ${install_prefix}/share/wax/test
      ./test_all.sh
    )
  fi
fi

#
# Dist
#
make dist
ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

tar xvfz wax-${VERSION}.tar.gz

if test $outofsource_only -eq 0; then
  ( cd wax-${VERSION}
    ./configure --prefix=$install_prefix
    ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi
    make uninstall
    make install
    ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

    if test -d $install_prefix/share/wax/test; then
      echo "@@@ Running tests from dist build..."
      ( cd $install_prefix/share/wax/test
        ./test_all.sh
        ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi
      )
    fi
  )
fi

#
# Out of source
#
( cd wax-${VERSION}
  if test -f src/Makefile; then
    make distclean
  fi
  ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi
  mkdir -p outofsource
)
( cd wax-${VERSION}/outofsource
  ../configure --prefix=$install_prefix
  make uninstall
  ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi
  make install
  ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

  if test -d $install_prefix/share/wax/test; then
    echo "@@@ Running tests from out-of-source build from dist..."
    ( cd ${install_prefix}/share/wax/test
      ./test_all.sh
    )
  fi
)
