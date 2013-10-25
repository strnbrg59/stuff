#
# Employee retention.  Works if /etc/passwd contains entries for all employees
# ever hired, and an employee's shell is /sbin/nologin if and only if he is
# gone.
#
# Produces a gnuplot commands file and a data file.
#
# Usage: "python gone-emps.py" (run on a machine that has the company-wide
# /etc/passwd file).  And then "gnuplot gone-emps.gp -" to see the plot.
#


#
# Set these constants as appropriate.
# Might also want to add more known-hire-dates in setup_spline().
#
min_id = 1001 # Minimum employee unix id

#
# end of user-setable constants
#

datafile_name = 'gone-emps.dat'
cmdsfile_name = 'gone-emps.gp'
bin_width = 100

def bin_index(id):
    return id/bin_width

def inv_bin_index(ndx):
    return ndx*bin_width

import sys
import scipy
from scipy import interpolate
import numpy

# We'll use cubic spline interpolation to estimate hire dates.
hire_date = None # will be a function of user id.
def setup_spline():
    known_hire_dates = numpy.array((
                        (1000, 2002.0),
                        (1676, 2008.75),
                        (1963, 2009.9),
                        (2512, 2010.9),
                        (3000, 2011.25),
                        (4000, 2012.7),
                        (4100, 2012.8),
                        (4200, 2012.85),
                        (4300, 2012.9),
                        (4400, 2012.95),
                        (4500, 2013.0),
                        (4600, 2013.05),
                        (4657, 2013.12),
                        (4750, 2013.33)))
                        # Add more if you know them.
                        # 
    return interpolate.interp1d(known_hire_dates[:,0],
                                known_hire_dates[:,1], 'cubic')

def hire_date_wrapper(id):
    try:
        return hire_date(id)
    except:
        print "Edit the setup_spline() function and add points going at least"
        print "as far as the highest current employee number."
        sys.exit(1)

def hire_date_str(id):
    return '%.1f' % hire_date_wrapper(id)

def time_employed(id, max_id):
    return hire_date_wrapper(max_id) - hire_date_wrapper(id)

def hazard_rate(pct_gone, id, max_id):
    return pct_gone/time_employed(id, max_id)

if __name__ == '__main__':
    hire_date = setup_spline()

    # Test the spline fit.  Could be screwy, especially if you
    # don't have good coverage of real observations over the entire
    # domain.
    if (len(sys.argv) == 2) and (sys.argv[1] == '-t'):
        for id in range(1000, 4658):
            if ((id % 10) == 0) or (id > 4650):
                print id, hire_date_wrapper(id)
        sys.exit(0)

    #
    # Data binning
    #
    bins = {}
    lines = open("/etc/passwd").readlines()
    max_id = int(lines[-1].split(":")[2])
    for line in lines:
        if line.find("/sbin/nologin") == -1:
           continue
        id = int(line.split(":")[2])
        if not (min_id <= id <= max_id):
            continue
    
        ndx = bin_index(id)
        if ndx in bins.keys():
            bins[ndx] += 1
        else:
            bins[ndx] = 1
    
    #
    # gnuplot commands
    #
    cmds = ("set title 'Employee Departure Analysis' font ',20'",
            "set style data histogram",
            "set style histogram cluster gap 1",
            "set style fill solid border -1",
            "set yrange [0:200]",
            "set xtics rotate by 90",
            "set xtics offset 0, character -9.5",
            "set xlabel 'employee number -- hire date' offset 1,2",
            "set ylabel 'percent'",
            "set tmargin at screen 0.85",
            "set bmargin at screen 0.25",
            "set terminal x11 size 1500,700",
            "plot '" + datafile_name + "' using 2:xtic(1) t 'gone', \
                   '' using 3:xtic(1) t 'annual hazard rate'")
    cmd_file = open(cmdsfile_name, 'w')
    for cmd in cmds:
        cmd_file.write(cmd + '\n')
    
    #
    # gnuplot data
    #
    data_file = open(datafile_name, 'w')
    for i in range(min(bins.keys()), max(bins.keys())):
        fmt = '"%d -- %s"'
        val = 0.0
        if i in bins.keys():
           val = bins[i]
        data_file.write(
            (fmt % (i*bin_width - min_id + 2, hire_date_str(inv_bin_index(i)))) +
            ' ' +
            str(val) + ' ' +
            str(100*hazard_rate((1.0*val)/bin_width, inv_bin_index(i), max_id)) + '\n')
