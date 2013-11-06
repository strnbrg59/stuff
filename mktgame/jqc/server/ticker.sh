#!/bin/sh
#
# ticker.sh: prints last few lines of the ticker, to a pipe.
#
# Usage: ticker.sh <assetName>

assetName=$1
tickerFile=data/ticker.out
tickerPipe=data/tickerPipe

echo "tickerPipe=$tickerPipe"
echo "tickerFile=$tickerFile"

grep $assetName $tickerFile | tail -20 | awk '{print $3,$4,$7,$8}' >> $tickerPipe 2>&1
