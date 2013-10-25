#!/bin/sh

echo '<html>'
echo '<BODY BGCOLOR="AAFFAA">'
echo '<CENTER><H2> Random Language Generator </H2></CENTER>'
echo '<form target="output_frame" action="/cgi-bin/ngrams/ngrams" method="GET">'
echo ''
echo '<TABLE BORDER=1 CELLPADDING=10 CELLSPACING=1>'
echo '<TR  BGCOLOR=#F0F000>'
echo '<TD>'
echo '<FONT SIZE=+2> training text (choose at least one) </FONT><BR>'
echo '<HR>'

# Make list of training texts.  They have to 
# be in a directory called training-texts, and they
# have to end in .txt, and they must not have any
# other periods ('.') in their names.
for i in training-texts/*.txt; do
    base=`basename $i | cut -d. -f1`
#   Too many people have been downloading my texts...
#   echo -n "<A HREF=\"`basename $i`\">"
    echo "$base</A><INPUT TYPE=\"checkbox\" NAME=\"training_`basename $i`\"><BR>"
done

echo '</TD>'
echo '</TR>'
echo ''
echo '<TR BGCOLOR=#6666FF>'
echo '<TD>'
echo '<FONT SIZE=+2>dependency depth</FONT>'
echo '<HR>'
echo '<SELECT NAME="depth" SIZE=1>'
echo '<OPTION> 1'
echo '<OPTION> 2'
echo '<OPTION SELECTED>3'
echo '<OPTION> 4'
echo '<OPTION> 5'
echo '<OPTION> 6'
echo '<OPTION> 7'
echo '<OPTION> 8'
echo '<OPTION> 9'
echo '<OPTION> 10'
echo '<OPTION> 11'
echo '</SELECT>'
echo '</TD>'
echo '</TR>'
echo ''
echo '</TABLE>'
echo ''
echo '<TABLE><TR BGCOLOR=#00F000><TD>'
echo '<input type="submit" value="submit">'
echo '</TD></TR></TABLE>'
echo '</form>'
echo ''
echo '</html>'
