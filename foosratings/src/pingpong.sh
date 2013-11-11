#!/bin/sh
HISTORY_LENGTH=10
GAMESFILE=pingpong.txt

TMPDIR=/tmp/$USER
mkdir -p $TMPDIR

./foosrater games_file=$GAMESFILE o_d_distinction=false history_length=$HISTORY_LENGTH debug_level=3 logistic_s=1000 sport="pingpong" weight_decay=1.00 > $TMPDIR/ppratings.html
echo
mv $TMPDIR/history.ps $TMPDIR/pphistory-unweighted.ps

: ${PUBLIC_HTML:=/u/tsternberg/public_html}; export PUBLIC_HTML
TARGET=$PUBLIC_HTML/pingpong.html

# Bar graph
sed "s'TMPDIR'$TMPDIR'" ppspread_against_best.gp > $TMPDIR/ppspread_against_best.gp
gnuplot $TMPDIR/ppspread_against_best.gp
convert -rotate 90 $TMPDIR/spread_against_best.gif $TMPDIR/spread_against_best90.gif
mv $TMPDIR/spread_against_best90.gif $PUBLIC_HTML/ppspread_against_best.gif

echo "<HTML><TITLE> Ping Pong Ratings </TITLE>" > $TARGET
echo "<H2><CENTER> Ping Pong Ratings </CENTER></H2>" >> $TARGET
cat $TMPDIR/ppratings.html >> $TARGET
echo "<BR><HR></BR>" >> $TARGET
echo "<img src="ppspread_against_best.gif" /img>" >> $TARGET
echo "<BR><HR></BR>" >> $TARGET

echo "<CENTER>ratings history, unweighted</CENTER>" >> $TARGET
convert -rotate 90 $TMPDIR/pphistory-unweighted.ps $TMPDIR/pphistory-unweighted.gif
cp $TMPDIR/pphistory-unweighted.gif $PUBLIC_HTML
echo "<img src="pphistory-unweighted.gif" /img>" >> $TARGET

dot -Tps $TMPDIR/graphviz.dat > $TMPDIR/graphviz.ps
convert $TMPDIR/graphviz.ps $TMPDIR/pp-graphviz.gif

# Once again, with weights
./foosrater games_file=$GAMESFILE o_d_distinction=false history_length=$HISTORY_LENGTH debug_level=3 logistic_s=1000 sport="pingpong" weight_decay=.99 > $TMPDIR/ppratings.html
echo
mv $TMPDIR/history.ps $TMPDIR/pphistory-weighted99.ps

echo "<BR><HR></BR>" >> $TARGET
echo "<CENTER>ratings history, weighted .99</CENTER>" >> $TARGET
convert -rotate 90 $TMPDIR/pphistory-weighted99.ps $TMPDIR/pphistory-weighted99.gif
cp $TMPDIR/pphistory-weighted99.gif $PUBLIC_HTML
echo "<img src="pphistory-weighted99.gif" /img>" >> $TARGET

echo "<BR><HR></BR>" >> $TARGET
cp $TMPDIR/pp-graphviz.gif $PUBLIC_HTML
echo "<img src="pp-graphviz.gif" /img>" >> $TARGET

echo "<BR><HR></BR>" >> $TARGET
cat ../web/theory.html >> $TARGET

echo "</HTML>" >> $TARGET
echo
