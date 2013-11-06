import sys
import os

def makeNewSpot():
    """
    Find the first unsaturated spot.
    Find the first spot, in its connections list, that is unsaturated.
    Insert new spot there (doing surgery on the lists).

    Return 0 if succeeded in making new spot.
    Return 1 if couldn't find even one unsaturated spot.
    Return 2 if couldn't find a second unsaturated spot.
    """
    spot1 = findFirstUnsaturatedSpot( g_spots.keys() )
    if spot1 == -1:
        return 1
    spot2 = findSecondUnsaturatedSpot( g_spots.keys(), spot1 )
    if spot2 == -1:
        return 2

    # Create a new spot and connect it to spot1 and spot2.
    new_spot = len(g_spots)
    g_spots[new_spot] = [spot1, spot2]
    g_spots[spot1].append( new_spot )
    g_spots[spot2].append( new_spot )

    return 0


def findFirstUnsaturatedSpot( spot_list ):
    """
    Unsaturated means, less than three connections.
    """
    for spot in spot_list:
        if len(g_spots[spot]) < 3:
            return spot
    return -1 # failure

def findSecondUnsaturatedSpot( spot_list, spot1 ):
    """
    Unsaturated means, less than three connections.
    Find an unsaturated spot, on the assumption that we're already going to use
    spot1 for our first spot.
    FIXME: For now, we will avoid connecting a spot to itself.
    """
    for spot in spot_list:
        if (spot!=spot1) and (len(g_spots[spot]) < 3):
            return spot
    return -1 # failure


def toPostscript( uniqizer ):
    dotfile_name =  "sprouts" + str(uniqizer) + ".dot"
    dotfile = open( dotfile_name, "w" )
    dotfile.write( 'digraph dots {\n' )
    drawn_connections = []
    for d in g_spots.keys():
        print "*** d=", d, " g_spots[d]=", g_spots[d]
        dotfile.write( '    ' + str(d) + ' -> {' )
        for e in g_spots[d]:
            if( (not (d,e) in drawn_connections)
            and (not (e,d) in drawn_connections)):
                drawn_connections.append((d,e))
                dotfile.write( str(e) + ' ' )
        dotfile.write( '}\n' )
    dotfile.write( '}\n' )
    dotfile.close()
    os.system( 'dot ' + dotfile_name + ' -Tps -o ' + dotfile_name + '.ps' )


if __name__ == '__main__':
    g_spots = {}
    for i in range(0, int(sys.argv[1])):
        g_spots[i] = [] # key=number,
                        # value=list of spots connected to this one.

    done = 0
    i = 0
    while done == 0:
        done = makeNewSpot()
        print "done=", done
        print "*** g_spots=", g_spots
        toPostscript(i)
        i += 1
