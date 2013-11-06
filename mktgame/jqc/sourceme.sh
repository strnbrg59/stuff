#!/bin/ksh
# Sets environment (at Persistence),
# starts rmiregistry and server for jqc (Berkeley Stock Exchange).
#
# Usage: you must source this file.
# Afterwards, your environment is prepared to support the running of the various
# jqc clients.

JAVA=/usr/local/tools/jdk1.1.5; export JAVA
CLASSPATH=".:$HOME/src/java:$JAVA/lib:$JAVA/lib/classes.zip"; export CLASSPATH
PATH="$JAVA/bin:$PATH"; export PATH

rmiregistry 19594&
( cd /home/teds/src/java/jqc/server 
  java jqc.server.Server &
)
