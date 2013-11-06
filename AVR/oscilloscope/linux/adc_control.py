import os
import subprocess
import time

g_serial_port_reader = None

def start(n_channels, delay_power):
    global g_serial_port_reader
    os.putenv("MYTTY", "/dev/ttyUSB1")

    # Kill any existing "serialport" processes, or they'll steal our's input.
    os.system("kill `ps aux |grep serialport | grep -v grep | awk '{print $2}'`\
              2> /dev/null")
    g_serial_port_reader = subprocess.Popen(args="serialport -b -r -s 38400",
                             shell=True, stdout=subprocess.PIPE)
    ret = os.system("serialport -s 38400 -x start")
    ret = os.system("serialport -s 38400 -x 0x" + ('%x'%(n_channels)).zfill(2))

    # Convert delay_power to two hex bytes.
    assert( 0 <= delay_power <= 16)
    ret = os.system("serialport -s 38400 -x " + "0x" +
                    ('%x'%(delay_power)).zfill(2))

def stop():
    ret = os.system("serialport -s 38400 -x stop")
    time.sleep(1) # Enough time for last batch to get dumped over uart.
    os.system("kill -s USR1 " + str(g_serial_port_reader.pid))


def makeGnuplotFile():
    outfilename = "/tmp/serial.txt"
    gpfilename = "/tmp/serial.gp"
    outfile = open(outfilename, "w")
    outfile.write(g_serial_port_reader.stdout.read())
    outfile.close()
    
    os.system("cat " + outfilename + " | ./adc2txt > " + gpfilename)
    return gpfilename


if __name__ == '__main__':
    import plots
    start(2,7)
    time.sleep(2)
    stop()
    gp_filename = makeGnuplotFile()
    plots.display(gp_filename, hang=True)
