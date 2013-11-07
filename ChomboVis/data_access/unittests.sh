#!/bin/sh

#
# Install this in libexec/ChomboVis, so there's no doubt as to where the
# *_unittest binaries are -- they're in the same directory.
#
# Run it from the directory it's in.

lib_dir=../../lib/ChomboVis
canonicals_dir=../../share/ChomboVis/test/canonicals
data_dir=../../share/ChomboVis/data
examples_dir=../../share/ChomboVis/examples
tmp_dir=/tmp/chombovis_$USER
if [ ! -x $tmp_dir ]; then mkdir $tmp_dir; fi

retval=0

# Need this for efence not to complain about how we clean up SaveToHDF5()
# file descriptors and other stuff:
export EF_ALLOW_MALLOC_0=1

#
# Iterate over the various tests.
#
test_name_list="ascii2hdf5_unittest ChomboHDF5_unittest ChomboHDF5_EBformattest \
                VisDat_unittest"
if test -f ChomboBridge_unittest; then
    test_name_list="$test_name_list ChomboBridge_unittest"
fi


for test_name in $test_name_list
do
  outfile=${test_name}.txt
  if   [ $test_name = "ChomboHDF5_unittest" ]; then
      infile=$data_dir/particles2D.hdf5
      executable=$test_name
  elif [ $test_name = "ChomboHDF5_EBformattest" ]; then
      infile=$data_dir/newebformat2d.hdf5
      executable=ChomboHDF5_unittest
  elif [ $test_name = "ChomboBridge_unittest" ]; then
      export MODULE_PATH=$lib_dir
      # Don't set LD_LIBRARY_PATH to where the Chombo libraries
      # have been installed.  It's not necessary, and in fact it's
      # downright harmful, because those libraries have the same
      # names.
      infile=$data_dir/newebformat2d.hdf5
      executable=$test_name
  elif [ $test_name = "VisDat_unittest" ]; then
      infile=$data_dir/particles3d.hdf5
      executable=$test_name
  elif [ $test_name = "ascii2hdf5_unittest" ]; then
      infile=$examples_dir/chomboascii.dat
      executable=${test_name}.sh
  fi

  export LD_PRELOAD=libefence.so.0
  ./$executable debug_level=4 infile=$infile > ${tmp_dir}/$outfile 2>&1
  if [ $test_name = "ascii2hdf5_unittest" ]; then
      retval=$?
  fi
  export LD_PRELOAD=
  if [ ! $test_name = "ascii2hdf5_unittest" ]; then
      cmp $canonicals_dir/$outfile ${tmp_dir}/$outfile > /dev/null 2>&1
      retval=$?
  fi
  if [ $retval -ne 0 ]; then
      echo "***Failed " $test_name \
          ", diff $canonicals_dir/$outfile ${tmp_dir}/$outfile="
      diff $canonicals_dir/$outfile ${tmp_dir}/$outfile
  else
      echo "***Passed $test_name."
  fi
done 