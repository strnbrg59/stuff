#!/bin/sh

./parking curb_length=2000 n_iterations=1 srand=0 > /tmp/test.out
diff /tmp/test.out ./canonical.out

