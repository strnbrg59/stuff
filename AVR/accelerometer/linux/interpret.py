"""
Reads and interprets accelerometer output produced by ../accel.cpp.
AVR must be on.
"""

import os
import sys
import subprocess
import random

def integrate(raw8bitVoltages, deltaT, one_g_response, simulate=False):
    """
    raw8bitVoltages: raw accelerometer readings on 0-255 scale.
    deltaT: seconds between readings.
    one_g_response: 8-bit value equal to 1g.

    Returns three tuples -- acceleration, speed and position.
    """
    one_g = 9.89 # meters/second^2
    accelScale = one_g/one_g_response # m/s^2 per acceleration unit

    accel = []
    for r in raw8bitVoltages:
        accel.append( (int(r) - raw8bitVoltages[0])*accelScale)
        # raw8bitVoltages[0] is the quiescent accelerometer reading.

    speed = [0,]
    distance = [0,]
    for t in range(1,len(accel)):
        speed.append(speed[t-1] + 0.5*deltaT*(accel[t-1] + accel[t]))
        distance.append(distance[t-1] + 0.5*deltaT*(speed[t-1] + speed[t]))


    if simulate:
        raw_sigma = 4 # of 8-bit voltage readings
        
        for i in range(0,100):
            t_raw = []
            for r in raw8bitVoltages:
                t_raw.append(r + int(random.gauss(r,raw_sigma)))
            t_a, t_s, t_d = integrate(t_raw, deltaT, one_g_response, simulate=False)
            print t_d[-1:][0]
    
    return accel, speed, distance


def plot(raw8bitVoltages, accel, speed, distance, deltaT):
    """ Output gnuplot-friendly data and display it. """
    outfilename = "/tmp/accel.gp"
    outfile = open(outfilename, 'w')
    outfile.write("channels = 4\n") # Expected by oscilloscope/linux/plots.py.

    # Create a dummy dataset; the oscilloscope plotting functions assume the
    # last dataset is the batch boundaries, and its lo/hi range is ignored when
    # deciding on how to present the plot and set the widget ranges.
    mins, maxes = [], []
    for dataset in raw8bitVoltages, speed, distance, accel:    
        mins.append(min(dataset))
        maxes.append(max(dataset))
    dummy = []
    for i in range(0,len(accel)-1):
        dummy.append(min(mins))
    dummy.append(max(maxes))

    for dataset in raw8bitVoltages, accel, speed, distance, dummy:
        for t in range(0,len(accel)):
            outfile.write("%9.2f"%(t*deltaT) + "%12d"%dataset[t] + '\n')
        outfile.write('\n')
    outfile.close()
    os.system("python ../../oscilloscope/linux/gui.py " + outfilename)


def loadData():
    datareader = subprocess.Popen(args="./eepromdump.sh",
                                  shell=True, stdout=subprocess.PIPE)
    raw8bitVoltages = []
    for a in datareader.stdout.read().split(','):
        if a == '0':
            break
        raw8bitVoltages.append(int(a))
    return raw8bitVoltages


def errorAnalysis(deltaT, one_g_response):
    raw8bitVoltages = loadData()
    integrate(raw8bitVoltages, deltaT, one_g_response, simulate=True)


if __name__ == '__main__':

    deltaT = 1/20.35 # attiny85 at 1MHz, prescaler /4096, OCR1A=12
    one_g_response = 64.5 # Device response to being turned 90 degrees
    if len(sys.argv) == 2:
        errorAnalysis(deltaT, one_g_response)
    
    # Read eeprom and convert strings to ints.
    raw8bitVoltages = loadData()

    (accel, speed, distance) = integrate(raw8bitVoltages, deltaT, one_g_response)
    plot(raw8bitVoltages, accel, speed, distance, deltaT)
