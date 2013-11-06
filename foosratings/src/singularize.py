import string
infile = open("realgames.txt")
for line in infile:
    items = line[:-1].split(' ')
    if items[0] == items[1]:
        reduced = (items[0], items[2], items[3], items[5])
        print string.join(reduced)
    else:
        print line[:-1]
