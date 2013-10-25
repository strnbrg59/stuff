import sys
import scipy
from scipy import interpolate
import numpy
import random

def gen_actuals():
    result = ([], [])
    prev = 0
    for i in range(0, 6):
        result[0].append(i)
        result[1].append(prev + random.uniform(0,10))
        prev = result[1][-1]
    return numpy.array(result[0]), numpy.array(result[1])

def gen_desired():
    result = []
    for i in range(0,100):
        result.append(i/20.0)
    return numpy.array(result)

if __name__ == '__main__':
    actuals = gen_actuals()
    desireds = gen_desired()
    print "desireds=", desireds
    f = interpolate.interp1d(actuals[0], actuals[1], 'cubic')

    actuals_file = open("actuals.dat", "w")
    for i in range(0, len(actuals[0])):
        actuals_file.write(str(actuals[0][i]) + ' ' + str(actuals[1][i]) + '\n')

    interps_file = open("interps.dat", "w")
    for x in desireds:
        interps_file.write(str(x) + ' ' + str(f(x)) + '\n')
