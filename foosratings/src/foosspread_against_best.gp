set terminal gif
set output "TMPDIR/spread_against_best.gif"
set grid
set yrange [0:5]
set style data histogram
set style fill solid border -1
set key off
set xtic rotate by 90 scale 0
set ytic rotate by 90 scale 0
plot 'TMPDIR/spread_against_best.gpd' using 2:xtic(1) title col
