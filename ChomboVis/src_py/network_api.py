#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

# File: network_api.py
# Author: TDSternberg
# Created: 12/02/2002

import anag_utils
from self_control import SelfControl
import os
import signal
import mutex


class NetworkApi( SelfControl ):
#Cut to here
    """ Master-slave communications """
#Cut from here
    def __init__( self, dep_dict ):
        """
        Convention: if we're not a master, then self.master=None, ditto for
        slave.
        """
        anag_utils.funcTrace()
        instance_vars = [
            {'name':'master'},
            {'name':'slave'},
            {'name':'fifo_path'},
            {'name':'slave_enabled', 'notify':1},
            {'name':'master_enabled', 'notify':1}
          ]
        SelfControl.__init__( self, dep_dict, instance_vars)

        self.fifo_path = anag_utils.g_temp_file_mgr.getTempDir() + '/fifo'

    #
    # Functions required by base class SelfControl.
    #
    def _initForFirstHDF5( self ):
        anag_utils.funcTrace()
    def _refresh( self ):
        anag_utils.funcTrace()
    def cleanup( self ):
        anag_utils.funcTrace()
#Cut to here


    def beSlave( self, on_off ):
        """
        When on_off==1, become a slave.
        When on_off==0, cease being a slave.

        A slave follows all the rotate, pan and zoom moves of its master; the
        two images move in unison.  However, the slave is still responsive to
        direct control, so it's possible to put the two images "out of phase"
        if desired.

        Do not start the slave process with "-i" on the ChomboVis command line,
        or the slave will not respond to master motions "instantly"; to make it
        respond you will need to move the mouse into the VTK window or hit
        <return> at the >>> prompt or some such.

        Process will hang until another ChomboVis calls beMaster().

        Easiest way to start a slave/master pair: use the 'cmd' command-line
        option to ChomboVis.
        $ chombovis cmd='c.network.beSlave(1)' foo.hdf5 &
        $ chombovis cmd='c.network.beMaster(1)' foo.hdf5
        
        If you cease being a slave, and want to become a slave again, you need
        to cycle the master process; call beMaster(0) then beMaster(1) on it
        (or do the equivalent in the GUI).

        It's possible to send commands to a slave straight from the shell (i.e.
        without starting a master ChomboVis.  Here's how.
        1. Start ChomboVis in slave mode (see above for how to do this).
        2. From the shell:
           a. $ slave_pid=`cat /tmp/chombovis_$USER/fifo`
           This will read the process id of the slave.  You can use ps if you
           forget the pid, later, but you must read that fifo if only
           to clear it.  If you don't have $USER defined in your environment,
           put your user id in there; in any event, there should, at this point,
           exist a FIFO that belongs to you, under /tmp/chombovis_<something>,
           and that's the FIFO you want to work with here. 

           b. echo any ChomboVis API command to the fifo.  For example
           $ newfile=turbulence.hdf5
           $ echo "c.reader.loadHDF5('$newfile')" > /tmp/chombovis_$USER/fifo
        
           c. Raise SIGUSR1 against the slave:
           $ kill -SIGUSR1 $slave_pid
           The slave ChomboVis should now execute the command.

        Example: share/ChomboVis/examples/newhdf5_into_slave.sh

        Caveats:
            1. Do not try running more than one ChomboVis slave at a time.
            2. Read the /tmp/chombovis_$USER/fifo exactly once -- no more, no
               less.
            3. Do not start the ChomboVis slave with "-i" on its command line
               (you can do that, but then it won't respond to commands unless
               you induce a refresh, e.g. move your mouse into the VTK window).
        """
        anag_utils.funcTrace()
        assert( on_off==0 or on_off==1 )

        self.setSlaveEnabled( on_off ) # notifier
        if on_off == 1:
            self.slave = Slave()
            self.slave.enable( self.fifo_path )
            self.vtk_vtk.setTopWindowTitle( "ChomboVis slave" )
        else:
            self.slave.disable()
            self.slave = None     # __del__() just doesn't get called!?
            self.vtk_vtk.setTopWindowTitle( "ChomboVis" )


    def beMaster( self, on_off ):
        """
        A master's rotate, pan and zoom operations are followed by its slave;
        the two images move in unison.

        Process will hang until another ChomboVis calls beSlave().

        Easiest way to start a slave/master pair: use the 'cmd' command-line
        option to ChomboVis.
        $ chombovis cmd='c.network.beSlave(1)' foo.hdf5 &
        $ chombovis cmd='c.network.beMaster(1)' foo.hdf5

        When on_off==1, become a master.
        When on_off==0, cease being a master.

        If you cease being a master, and want to become a master again, you need
        to cycle the slave process; call beSlave(0) then beSlave(1) on it
        (or do the equivalent in the GUI).
        """
        anag_utils.funcTrace()
        assert( on_off==0 or on_off==1 )
        
        self.setMasterEnabled( on_off ) # notifier
        if on_off == 1:
            self.master = Master()
            if 0 == self.master.enable( self.fifo_path ):
                self.vtk_vtk.setTopWindowTitle( "ChomboVis master" )
                self.sendCommand( 'pass' ) # Master doesn't come up w/o this.
            else: # failure
                self.setMasterEnabled( 0 )
                self.master = None
        else:
            self.master.disable()
            self.master = None
            self.vtk_vtk.setTopWindowTitle( "ChomboVis" )


    def sendCommand( self, cmd ):
        """
        Send a command from the master to the slave.
        """
        anag_utils.funcTrace()
        if not self.master:
            return
        else:
            self.master.sendCommand( cmd )
    

#Cut from here
class Master:
    def __init__( self ):
        anag_utils.funcTrace()

    def enable( self, fifo_path ):
        """
        Returns 0 on success, 1 on error.
        """
        anag_utils.funcTrace()
        self.fifo_path = fifo_path
        if os.path.exists( fifo_path ):
            # Pick up slave pid.
            f = open( fifo_path, 'r' )
            self.slave_pid = int( f.readline() )
            f.close()
            return 0

        else:
            anag_utils.error( 'Put slave process into slave mode first, and '
                              'only then start master.' )
            return 1

    def disable( self ):
        anag_utils.funcTrace()
        # We'll want the master to know if it's enabled, so that, when lots
        # of api's call the master to send a command, the master knows not to
        # waste its time.


    def sendCommand( self, cmd ):
        anag_utils.funcTrace()
        f = open( self.fifo_path, 'w+' )
        f.write( cmd + '\n' )
        f.close()
        try:
            os.kill( self.slave_pid, signal.SIGUSR1 )
        except:
            anag_utils.error( "Slave process is not responding." )


class Slave:
    """
    Owns the FIFO.
    """
    def __init__( self ):
        signal.signal( signal.SIGUSR1, signal.SIG_IGN )
        self.mewtex = mutex.mutex()

    def __del__( self ):
        """
        Can't count on this firing off if we just set slave to None.  Hence
        self.disable().
        """
        anag_utils.funcTrace()
        self.disable()


    def disable( self ):
        anag_utils.funcTrace()
        if os.path.exists( self.fifo_path ):
            self.fifo.close()
            signal.signal( signal.SIGUSR1, signal.SIG_IGN )


    def enable( self, fifo_path ):
        anag_utils.funcTrace()

        self.fifo_path = fifo_path
        if not os.path.exists( fifo_path ):
            os.mkfifo( fifo_path, 0600 )

        # Handshake procedure:
        # 1. Slave writes its pid into the fifo.
        # 2. Master reads that pid from the fifo.
        # 3. Slave closes fifo, then reopens it for reading.
        # 4. Master closes fifo, then reopens it for writing.
        self.fifo = open( fifo_path, 'w' ) # Blocks, awaiting master
        self.fifo.write( str( os.getpid() ) )
        self.fifo.close()

        self.fifo = open( fifo_path, 'r' )
        signal.signal( signal.SIGUSR1, self.signalHandler )


    def signalHandler( self, signum, frame ):
        self.mewtex.lock( self.signalHandlerImpl, 'dummy' )


    def signalHandlerImpl( self, dummy ):
        import chombovis
        c = chombovis.latest()
        for cmd in self.fifo.readlines():
            # Nothing will be there to read, until the write side closes its
            # handle to the fifo.
            exec( cmd )
        self.mewtex.unlock()
