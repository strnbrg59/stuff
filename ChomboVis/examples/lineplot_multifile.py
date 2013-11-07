#
# ChomboVis script that produces lineplot data in gnuplot- and matlab-ready
# files.  Loops over any number of hdf5 files.
#
# For more info on ChomboVis lineplots, see
# http://seesar.lbl.gov/anag/chombo/chomboVis/vtk_line_plot.html
#
# Edit the following parameters to match your needs.  Then, at the shell prompt
# type this: "chombodatalite lineplot_multifile.py"
#

p0 = (0.0, 0.1  ) # Starting point for line (use 3 components if 3D file)
p1 = (1.0, 0.12 ) # Ending point for line (use 3 components if 3D file)

points_per_line = 5
component = 0         # Field data component number
max_level = 0         # Maximum level of refinement to look at
filename_pattern = "shock*.hdf5"    # Same syntax understood by /bin/ls
gnuplot_outfilename = "multi.gp"    # Gnuplot-style output (will be created).
matlab_outfilename = "multi.txt"    # Matlab-style output (will be created).

######################## End of user-editable stuff #######################
import visualizable_dataset
import sys
import glob

all_files = glob.glob( filename_pattern )
print "all_files=", all_files
lineplots = []
for filename in all_files:
    v = visualizable_dataset.VisualizableDataset( hdf5filename=filename )
    lineplot = v.getLinePlot(p0,p1,points_per_line,component,max_level)
    lineplots.append( lineplot )

#
# Print in two different formats
#
print "Printing gnuplot-style file..."
gnuplot_outfile = file( gnuplot_outfilename, 'w' )
for plot in lineplots:
    for row in plot:
        gnuplot_outfile.write( str(row[0]) + ' ' + str(row[1]) + '\n' )

print "Printing matlab-style file..."
matlab_format = []
data0 = lineplots[0]
for row in data0:
    matlab_format.append( list(row) )
for data in lineplots[1:]:
    r = 0
    for row in data:
        matlab_format[r].append( row[1] )
        r += 1
matlab_outfile = file( matlab_outfilename, 'w' )
for row in matlab_format:
    for item in row:
        matlab_outfile.write( str(item) + ' ' )
    matlab_outfile.write( '\n' )

sys.exit(0)
