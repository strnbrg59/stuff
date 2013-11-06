#!/bin/sh

# Scan through the cpp and hpp files and, based on their #include lines (for files
# not in angle brackets) produce Makefile-style rules.

for i in *.cpp; do
    base=`echo $i | cut -d. -f1`
    echo -n "${base}.o : $i "
    echo -n `grep '#include' $i | grep -v '<[A-z]' | sed 's/#include//g' | tr "\"" " "`
    echo
done

for i in *.hpp; do
    echo -n "$i : "
    export tempp=`grep '#include' $i | grep -v '<[A-z]' | sed 's/#include//g' | tr "\"" " "`
    test -n "$tempp"
    if [ $? -eq 0 ]; then
        echo $tempp
        echo "@touch $i" | tr "@" "\t"
    else
        echo
    fi
done