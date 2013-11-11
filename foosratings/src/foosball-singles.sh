#!/bin/sh
HISTORY_LENGTH=20
GAMESFILE=realgames.txt
SINGLESFILE=foosball-singles.txt # Generated!  Don't edit!

TMPDIR=/tmp/$USER
mkdir -p $TMPDIR

cat $GAMESFILE | awk 'NF==4' > $SINGLESFILE

./foosrater games_file=$SINGLESFILE o_d_distinction=false history_length=$HISTORY_LENGTH debug_level=3 logistic_s=1000 sport="foosball" weight_decay=1.00 > ${TMPDIR}/foosratings.html
echo
mv ${TMPDIR}/history.ps ${TMPDIR}/fooshistory-unweighted.ps

PUBLIC_HTML=/u/tsternberg/public_html/$TEMP_HTML
TARGET=$PUBLIC_HTML/foosball.html

# Bar graph
sed "s'TMPDIR'$TMPDIR'" foosspread_against_best.gp > $TMPDIR/foosspread_against_best.gp
gnuplot $TMPDIR/foosspread_against_best.gp
convert -rotate 90 $TMPDIR/spread_against_best.gif $TMPDIR/spread_against_best90.gif
mv $TMPDIR/spread_against_best90.gif $PUBLIC_HTML/foosspread_against_best.gif

echo "<HTML><TITLE> Foosball Singles Ratings </TITLE>" > $TARGET
cat $TMPDIR/foosratings.html >> $TARGET
echo "<BR><HR></BR>" >> $TARGET
echo "<img src="foosspread_against_best.gif" /img>" >> $TARGET
echo "<BR><HR></BR>" >> $TARGET

echo "<CENTER>ratings history, unweighted</CENTER>" >> $TARGET
convert -rotate 90 $TMPDIR/fooshistory-unweighted.ps $TMPDIR/fooshistory-unweighted.gif
cp $TMPDIR/fooshistory-unweighted.gif $PUBLIC_HTML
echo "<img src="fooshistory-unweighted.gif" /img>" >> $TARGET

dot -Tps $TMPDIR/graphviz.dat > $TMPDIR/graphviz.ps
convert $TMPDIR/graphviz.ps $TMPDIR/foos-graphviz.gif

# Once again, with weights
./foosrater games_file=$GAMESFILE o_d_distinction=false history_length=$HISTORY_LENGTH debug_level=3 logistic_s=1000 sport="foosball" weight_decay=.99 > $TMPDIR/foosratings.html
echo
mv $TMPDIR/history.ps $TMPDIR/fooshistory-weighted99.ps

echo "<BR><HR></BR>" >> $TARGET
echo "<CENTER>ratings history, weighted .99</CENTER>" >> $TARGET
convert -rotate 90 $TMPDIR/fooshistory-weighted99.ps $TMPDIR/fooshistory-weighted99.gif
cp $TMPDIR/fooshistory-weighted99.gif $PUBLIC_HTML
echo "<img src="fooshistory-weighted99.gif" /img>" >> $TARGET

echo "<BR><HR></BR>" >> $TARGET
cp $TMPDIR/foos-graphviz.gif $PUBLIC_HTML
echo "<img src="foos-graphviz.gif" /img>" >> $TARGET

echo "<BR><HR></BR>" >> $TARGET
cat ../web/theory.html >> $TARGET

echo "</HTML>" >> $TARGET
echo

rm $SINGLESFILE
