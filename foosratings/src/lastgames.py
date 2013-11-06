#!/usr/bin/env python

import sys
import random

# How many of the last game results to display
lastGamesToDisplay = 30

# How many consecutive wins/losses to count as a streak
streakThreshhold = 4


# Reads the game info file, returns a dict of the form:
# { 'players' : { 'playerName' : { 'wins' : int,
#                                  'losses' : int,
#                                  'currentWinStreak' : int (negative for losses),
#                                  'maxWinStreak' : int,
#                                  'maxLossStreak' : int }
#               }
#   'games' : [ { 'winner' : string,
#                 'loser' : string,
#                 'margin' : int,
#                 'description' : string } ]
# }
def readGames(filename):
    gameInfo = {}
    gameInfo['games'] = []
    gameInfo['players'] = { }

    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    for line in lines:
        # strip newlines/extra spaces
        line = line.strip()

        # should be "winner score loser score"
        data = line.split(' ')

        if len(data) != 4:
            print 'Cannot parse "%s"' % line
            continue

        thisGame = []

        winner = data[0]
        loser = data[2]
        winnerPoints = int(data[1])
        loserPoints = int(data[3])

        thisGame = { 'winner' : winner,
                     'loser' : loser,
                     'margin' : winnerPoints - loserPoints }

        # Make sure each player exists

        for player in [ winner, loser ]:
            players = gameInfo['players']

            if player not in players:
                players[player] = { }
                players[player]['wins'] = 0
                players[player]['losses'] = 0
                players[player]['currentWinStreak'] = 0
                players[player]['maxWinStreak'] = 0
                players[player]['maxLossStreak'] = 0

        gameDesc = winDescription(winner, loser, winnerPoints, loserPoints)

        winnerEndsLossStreak = False
        loserEndsWinStreak = False

        if players[winner]['currentWinStreak'] <= -streakThreshhold:
            winnerEndsLossStreak = True

        if players[winner]['currentWinStreak'] >= streakThreshhold:
            loserEndsWinStreak = True

        # Update the win/losses
        players[winner]['wins'] += 1
        players[loser]['losses'] += 1

        if players[winner]['currentWinStreak'] < 0:
            players[winner]['currentWinStreak'] = 0
        if players[loser]['currentWinStreak'] > 0:
            players[loser]['currentWinStreak'] = 0

        players[winner]['currentWinStreak'] += 1
        players[loser]['currentWinStreak'] -= 1

        if players[winner]['currentWinStreak'] > players[winner]['maxWinStreak']:
            players[winner]['maxWinStreak'] = players[winner]['currentWinStreak']

        if players[loser]['currentWinStreak'] < players[loser]['maxLossStreak']:
            players[loser]['maxLossStreak'] = players[loser]['currentWinStreak']


        if winnerEndsLossStreak:
            gameDesc += ', ending %s\'s losing streak!' % winner

        elif players[winner]['currentWinStreak'] >= streakThreshhold:
            gameDesc += '. %s is on a winning streak! (%d)' \
                        % (winner, players[winner]['currentWinStreak'])

        elif loserEndsWinStreak:
            gameDesc += ', ending %s\'s winning streak!' % loser

        elif players[loser]['currentWinStreak'] <= -streakThreshhold:
            gameDesc += ', extending %s\'s losing streak to %d' \
                        % (loser, -players[loser]['currentWinStreak'])

        # Update the game

        thisGame['description'] = gameDesc

        gameInfo['games'].append(thisGame)

    return gameInfo


# Make a fun description for the game
def winDescription(winner, loser, winnerPoints, loserPoints):

    margin = winnerPoints - loserPoints

    if winnerPoints > 21:
        # Went into deuce!
        descs = [ '%(winner)s defeated %(loser)s in an epic struggle',
                  '%(winner)s defeated %(loser)s in a titanic clash' ]

    elif margin in range(19, 999):
        descs = [ '%(winner)s annihilated %(loser)s',
                  '%(winner)s utterly destroyed %(loser)s'
                ]

    elif margin in range(15, 19):
        descs = [ '%(winner)s humiliated %(loser)s',
                  '%(winner)s clobbered %(loser)s',
                  '%(winner)s crushed %(loser)s'
                ]

    elif margin in range(11, 15):
        descs = [ '%(winner)s defeated %(loser)s in a one-sided contest',
                  '%(winner)s spanked %(loser)s in a painful display',
                  '%(winner)s kicked %(loser)s\'s butt',
                  '%(winner)s left skid marks all over %(loser)s\'s back',
                  '%(winner)s left some doubt as to whether %(loser)s has an opposable thumb',
                  '%(winner)s left %(loser)s wishing there were no witnesses',
                  '%(winner)s gave %(loser)s a lesson he will not soon forget',
                ]

    elif margin in range(7, 11):
        descs = [ '%(winner)s reminded %(loser)s that keeping his day job would be a good idea',
                  '%(winner)s easily defeated %(loser)s',
                  'After this loss to %(winner)s, %(loser)s is all out of excuses'
                ]

    elif margin in range(4, 7):
        descs = [ '%(winner)s defeated %(loser)s',
                  '%(loser)s came up short against %(winner)s',
                  '%(loser)s put up a good fight against %(winner)s'
                ]
    else:
        descs = [ '%(winner)s barely defeated %(loser)s',
                  '%(winner)s held his own against %(loser)s',
                  '%(loser)s was narrowly defeated by %(winner)s' ]

    chosenDesc = descs[random.randint(0, len(descs) - 1)]
    tempDict = { 'winner' : winner, 'loser' : loser }

    description = chosenDesc % tempDict
    description += ' (%d - %d)' % (winnerPoints, loserPoints)

    return description


# Given a gameInfo dict as provided by readGames, print out stats for all
# players
def describePlayerStats(gameInfo):
    print '<TABLE BORDER=1 CELLPADDING=10 CELLSPACING=1>'

    print '  <TR>'
    print '    <TD>Player</TD>'
    print '    <TD>Wins</TD>'
    print '    <TD>Losses</TD>'
    print '    <TD>Current Winning Streak</TD>'
    print '    <TD>Max Winning Streak</TD>'
    print '    <TD>Max Losing Streak</TD>'
    print '  </TR>'

    playerNames = gameInfo['players'].keys()
    playerNames.sort()

    for player in playerNames:
        playerInfo = gameInfo['players'][player]

        print '  <TR>'
        print '    <TD>%s</TD>' % player
        print '    <TD>%d</TD>' % playerInfo['wins']
        print '    <TD>%d</TD>' % playerInfo['losses']
        print '    <TD>%d</TD>' % playerInfo['currentWinStreak']
        print '    <TD>%d</TD>' % playerInfo['maxWinStreak']
        print '    <TD>%d</TD>' % playerInfo['maxLossStreak']
        print '  </TR>'

    print '</TABLE>'


# Given a gameInfo dict as provided by readGames, print out the last N games.
# if lastN is < 0, prints them all.
def describeLastGames(gameInfo, lastN):
    games = gameInfo['games'][-lastN:]
    for game in games:
        print '<p>' + game['description']


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'No games filename given'
        sys.exit(1)

    gameInfo = readGames(sys.argv[1])
    describePlayerStats(gameInfo)

    print '<p><hr><p>'
#    describeLastGames(gameInfo, lastGamesToDisplay)
