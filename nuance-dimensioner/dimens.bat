echo off

if not exist Dimens.jar goto houston_we_have_a_problem

echo Starting the Dimensioner - please wait...
java -cp Dimens.jar Dimensioner
goto end

:houston_we_have_a_problem

echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
echo @
echo @  The Dimensioner failed to start up.
echo @
echo @  Please make sure you have extracted the entire contents
echo @  of the ZIP file into a directory and are running (double-
echo @  clicking) this batch file in that directory.  You haven't
echo @  really extracted the files until you can see them with
echo @  Windows Explorer (or dir in an MSDOS window).
echo @  
echo @  You can invoke this bat file from an MSDOS window or from
echo @  Windows Explorer.  But you should not invoke it from a
echo @  web browser (you will get this message if you try).
echo @
echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

pause

:end

