#!/bin/sh
GAMESFILE=foos2013.txt
TABLEFILE=foosdoubles.html

TMPDIR=/tmp/$USER
mkdir -p $TMPDIR

./foosrater games_file=$GAMESFILE history_length=0 debug_level=3 logistic_s=1000 sport="foosball" hasima=4 > $TMPDIR/$TABLEFILE

PUBLIC_HTML=/u/tsternberg/public_html
TARGET=$PUBLIC_HTML/$TABLEFILE

# Bar graph
egrep -v '^<|total|rating' $TMPDIR/$TABLEFILE | awk '{print $1, $3, $4}' > foosdoubles_bars.out
gnuplot foosdoubles.gp
rm foosdoubles_bars.out
mv foosdoubles.gif $PUBLIC_HTML

echo "<HTML><TITLE> Foosball Doubles Ratings </TITLE>" > $TARGET
cat $TMPDIR/$TABLEFILE >> $TARGET
echo "<BR><HR></BR>" >> $TARGET
echo "<img src="foosdoubles.gif" /img>" >> $TARGET
echo "<BR><HR></BR>" >> $TARGET

echo "<BR><HR></BR>" >> $TARGET
cat ../web/theory_doubles.html >> $TARGET

echo "</HTML>" >> $TARGET
echo
