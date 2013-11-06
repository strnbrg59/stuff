#!/bin/sh

# Generates GrammarDatabase.java from a template file, plus a list
# of grammars that get inserted into Java addElements() calls.

SOURCE=GrammarDatabase.java-template
TARGET=GrammarDatabase.java
DATA=grammars.dat

TEMPFILE=/tmp/temp.$$ #FIXME: make this portable to Win32!
if [ -f $TEMPFILE ]; then rm -f $TEMPFILE; fi

cat $SOURCE | sed '/addElements starting here/q' > $TEMPFILE
cat $DATA              |\
     sed 's/#.*$//g'   |\
     grep -v '^$'      |\
     sed 's/  */ /g'   |\
     awk -F! '{printf "m_rep.addElement(new Grammar(%s,%s,%s));\n",$1,$2,$3}' \
        >> $TEMPFILE
cat $SOURCE | sed -n '/end of addElements section/,1000p' >> $TEMPFILE

mv -f $TEMPFILE $TARGET
