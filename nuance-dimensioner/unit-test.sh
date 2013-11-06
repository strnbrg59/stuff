#!/bin/sh

# Run the main() methods of all classes that have them (except
# Dimensioner itself) and compare the output to a known correct 
# output file.

# Test AnalyticalBackEnd
java AnalyticalBackEnd > tester.new 2>&1
diff tester.correct-output tester.new
rm tester.new

