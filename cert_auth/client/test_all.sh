#!/bin/sh

outfile=test_all.out
correct=test_all.correct

#
# Run the tests and capture the output.
#
cert_dir=/tmp/cert_auth
rm -rf $cert_dir

# A few negative tests.
./show_own_certificate.sh   > $outfile
./sign_certificate.sh      >> $outfile

# Positive tests
./generate_own_key_pair.sh >> $outfile
./accept_key_pair.sh       >> $outfile
./show_own_certificate.sh  >> $outfile
./sign_certificate.sh      >> $outfile

for f in `ls $cert_dir`; do
    echo "*** $f ***" >> $outfile
    cat $cert_dir/$f  >> $outfile
done

#
# Compare actual output to correct output.
#
cmp $correct $outfile > /dev/null
if [ $? -eq 0 ]; then
    echo "success"
    rm $outfile
else
    echo "failure: compare $outfile against $correct"
fi
