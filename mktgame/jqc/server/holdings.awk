# Print running record of net holdings.
# Usage: cat ticker.out | awk -f holdings.awk trader="trader-name"

$1==trader {units += $3; money -= $3 * $8; printf "%45s %8d %8.1f\n", $0, units, money;}
$6==trader {units -= $3; money += $3 * $8; printf "%45s %8d %8.1f\n", $0, units, money;}
