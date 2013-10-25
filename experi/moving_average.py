import sys
import random

def moving_average(ts, lag) :
    assert(len(ts) > lag)
    result = []

    first_val = 0
    for t in range(0, lag):
        first_val += ts[t]/(lag + 0.0)
    result.append(first_val)
    for t in range(lag, len(ts)):
        result.append(result[t-lag] + (ts[t] - ts[t-lag])/(lag + 0.0))

    return result

#
# Example:
# # python moving_average.py 100 5 > gp.dat
# # gnuplot
# gnuplot> plot "gp.dat" u 1:2 w linespoints, "gp.dat" u 1:3 w linespoints
#
if __name__ == '__main__':
    n = 0
    lag = 0
    try:
        n = int(sys.argv[1])
        lag = int(sys.argv[2])
        assert(n > lag)
    except:
        print "Usage:", sys.argv[0], "series-length lag"
        sys.exit(1)

    # Generate a random-walk-type time series.
    ts = []
    ts.append(random.random() - 0.5)
    for t in range(1, n):
        ts.append(ts[t-1] + random.random() - 0.5)
    
    # Take a moving average.
    ma = moving_average(ts, lag)

    # Print out ts and ma, lined up so it looks like central averages.
    for t in range(lag/2, n+lag/2-lag):
        print t, ts[t], ma[t-lag/2+1]
