#! /bin/bash

# Give everybody the same amount of money.
# Usage: endow.sh <amount>
# Must be launched in the jqc/server directory.

######################
# Check prerequisites
if [ $# -ne 1 ]; then
	echo "Usage: $0 <amount>"
	exit 1
fi

if [ ! -f passwords.dat ]; then
	echo "File does not exist: passwords.dat"
	exit 1
fi
######################

amount=$1
cat passwords.dat | grep -v '^#' | grep '[a-zA-Z0-9]=[a-zA-Z]' > /tmp/jqc.passwords

while read line; do
	trader=`echo $line | cut -d= -f1`
	echo "Fed gave $amount money to $trader" >> data/ticker.out
done < /tmp/jqc.passwords
