#!/bin/sh

tmp_dir=/tmp
text_outfile_suffix=test

for test_name in darkframe yes no deneb
do
    case $test_name in
        darkframe ) ppm_outfile=trim_scaling_2.ppm.bak
                    dark_frame_option='dark_frame=../data/dark_frame.ppm.gz' ;;
        yes )       ppm_outfile=trim_scaling_2.ppm.bak
                    dark_frame_option= ;;
        no )        dark_frame_option= ;;
        deneb )     ppm_outfile=trim_scaling_11.ppm.bak
                    dark_frame_option=
                    echo "Long test now..." ;;
    esac

    ../../../bin/astro debug_level=4 data_dir=../data/ \
                       data_suffix=_${test_name}.ppm.gz \
                       max_miss_ratio=0.05  $dark_frame_option \
                       > ${tmp_dir}/${test_name}_${text_outfile_suffix}.txt 2>&1
    
    # Compare text output to canonical
    cmp ./canonicals/${test_name}_${text_outfile_suffix}.txt \
        ${tmp_dir}/${test_name}_${text_outfile_suffix}.txt
    if test $? -eq 0 ; then
        echo "*** Passed ${test_name}-test text test."
    else
        echo "*** Failed text test."\
             "tkdiff ${tmp_dir}/${test_name}_${text_outfile_suffix}.txt "\
             "./canonicals/${test_name}_${text_outfile_suffix}.txt"
    fi
    
    # Compare ppm output to canonical.
    if test $test_name = "no" ; then continue; fi
    retval=0
    cmp ./${ppm_outfile} canonicals/sum_${test_name}.ppm
    if test $? -ne 0 ; then
        echo "!!! Failed image-dump test.  If you have "
        echo "/usr/X11R6/bin/composite, we will now use it to subtract"
        echo "./${ppm_outfile} from "\
             "canonicals/sum_${test_name}.ppm."
        echo "If you see a mostly black image, then you are all right."
        retval=`expr $retval + 1`
    
        which composite > /dev/null
        have_composite=$?
        which xv > /dev/null
        have_xv=$?
        if test  $have_composite -eq 0 -a $have_xv -eq 0 ; then
            composite -compose difference \
                canonicals/sum_${test_name}.ppm \
                ./${ppm_outfile} ${tmp_dir}/diffs.ppm
            xv ${tmp_dir}/diffs.ppm
        fi
    else
        echo "*** Passed ${test_name}-test image-dump test."
    fi
done
