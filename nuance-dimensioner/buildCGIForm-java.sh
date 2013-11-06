#!/bin/sh

if [ $# -ne 1 ]; then
    echo "========================================"
    echo "Usage: $0 [intranet|extranet]"
    echo ""
    echo "Creates CGIForm.java, the program that produces what the user"
    echo "thinks of as the dimensioner GUI."
    echo "Uses two inputs: a template of CGIForm.java, plus a text file,"
    echo "cgiForm-intranet.html or cgiForm-extranet."
    echo 'cgiForm-*net.html is transformed as described below'
    echo "and then inserted into CGIForm.java-template, right after the"
    echo "text that reads \" // -- addHTML starting here\" "
    echo ""
    echo 'Lines of cgiForm-*net.html are, for the most part, just enclosed within'
    echo "double quotes and System.out.println().  However, it also understands"
    echo "three escape sequences:"
    echo '  1. Lines beginning with # are comments; they are discarded.'
    echo '  2. Text within a pair of !| and |! is understood to be the name'
    echo '     of a program variable or something that has a toString() method,'
    echo '     i.e. something that will print inside a System.out.println().'
    echo '     However, we cannot handle more than one set of !|...|! on a'
    echo '     single line.'
    echo '  3. Lines beginning with !! are passed through with no modifications'
    echo '     (except the ^!! is removed).  Such lines are presumably embedded'
    echo '     code.'
    echo '========================================'
fi

SOURCE=CGIForm.java-template
TARGET=CGIForm.java
DATA=cgiForm-${1}.html

PART1=/tmp/temp1.$$
PART2=/tmp/temp2.$$
PART3=/tmp/temp3.$$
EMBEDDED_CODE=/tmp/embedded_code.java.$$

if [ -f $PART1 ]; then rm -f /tmp/temp*; fi

# The part before  // -- addHTML starting here
cat $SOURCE | sed '/addHTML starting here/q' > $PART1

#
# Modifications to cgiForm.html.  See explanation at top of this file."
#
while read line; do
    echo $line | grep -v '^#' | grep ^!! > /dev/null
    if [ $? -eq 0 ]; then
        echo $line | grep -v '^#' | sed 's/^\!\!//'
    else
        echo $line | grep -v '^#' | grep '!|' > /dev/null
        if [ $? -eq 0 ]; then
            echo $line | grep -v '^#' | sed 's/"/\\"/g' |\
              sed 's/^\(.*\)\(!|.*|!\)\(.*\)$/CGIForm.println("\1"+\2+"\3");/' |\
              sed 's/!|//' | sed 's/|!//'
        else
            echo $line | grep -v '^#' | sed 's/"/\\"/g' |\
              sed 's/^\(.*\)$/CGIForm.println("\1");/'
        fi
    fi
done < $DATA > $PART2
#cat $PART2

# The part after // -- end of addHTML section
cat $SOURCE | sed -n '/end of addHTML section/,1000p' > $PART3

cat $PART1 $PART2 $PART3 > $TARGET



