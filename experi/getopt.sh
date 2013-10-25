#!/bin/sh

while getopts "s:u:i:arlm:n:t:" options; do
    case $options in
        u ) OPT_URL=1; C_URL=$OPTARG ;;
        s ) OPT_SECRET=1; C_SECRET=$OPTARG ;;
        i) OPT_FILE=1; C_FILE=$OPTARG; echo "Option i=$C_FILE" ;;
        m ) OPT_MODEL=1; C_MODEL=$OPTARG ;;
        n ) OPT_SERIALNUM=1; C_SERIALNUM=$OPTARG ;;
        t ) OPT_MAXAPPLIANCES=1; C_MAXAPPLIANCES=$OPTARG ;;
        a ) OPT_AUTOMATED=1 ;;
        r ) OPT_RESTORE=1 ;;
        l ) OPT_LICENSEPROMPT=1 ;;
        * ) echo "Bad option" ;;
    esac
done

case Yes in
    y*|Y* ) echo "wy" ;;
    * ) echo "not matched" ;;
esac
