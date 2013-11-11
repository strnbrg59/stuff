set terminal gif
set output "foosdoubles.gif"
set grid x
set grid y
set style data histogram
set style fill solid border -1
set ylabel 'rating'
set xtics rotate by -45 scale 0
plot 'foosdoubles_bars.out' using 3:xtic(1) title 'defense', '' using 2:xtic(1) title 'offense'
