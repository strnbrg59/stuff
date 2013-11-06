// Convert the AVR adc.cpp program's output to something human-readable.
// See ../adc.cpp for description of data format.
//
// This program reads from stdin and writes to stdout.
//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <stdint.h>
#include <algorithm>

int main()
{
    //
    // Read metadata.
    //
    uint8_t uCMHz, nChannels;
    uint16_t batchSize;
    read(STDIN_FILENO, &uCMHz, 1);
    read(STDIN_FILENO, &nChannels, 1);
    read(STDIN_FILENO, &batchSize, 2);

    printf("channels = %d\n", nChannels);

    //
    // Read batches of known size.  Throw away last, incomplete, batch.
    // In input, channel is the faster-changing index, but in output it's the
    // opposite; for gnuplot we want each channel's data, for each batch, to
    // look like a separate dataset.
    //
    uint8_t voltages[batchSize][nChannels];
    double eventTime = 0;
    int vertlo=-20, verthi=265;
    int nb;
    bool printedOnce=false;

    for(;;)
    {
        // Timing info for AD conversions.
        uint16_t ticks;
        nb = read(STDIN_FILENO, &ticks, 2);
        if( nb == 0 ) exit(0);
        double deltaT = (256.0*ticks)/(batchSize*uCMHz*1E6);
        if(!printedOnce)
        {
            fprintf(stderr, "bandwidth=%f KHz\n", 1/(deltaT*1000));
            printedOnce = true;
        }
        // 256 = AVR timer prescaler

        for( int b=0; b<batchSize; ++b )
        {
            for( int c=0; c<nChannels; ++c )
            {
                nb = read(STDIN_FILENO, &(voltages[b][c]), 1);
                if( nb == 0 ) exit(0);
            }
        }

        //
        // Output.
        //
        for( int c=0;c<nChannels;++c )
        {
            for( int b=0;b<batchSize;++b )
            {
                eventTime += deltaT;
                printf("%10.7f %d\n", eventTime, int(voltages[b][c]));
            }

            // Go back to time of first reading of next channel.
            if( c < nChannels-1 )
            {
                eventTime -= (deltaT*batchSize) + deltaT;
            }

            // A blank line -- so gnuplot interprets each channel's data as a
            // separate dataset:
            printf("\n");
        }

        // Timing info for uart transmission
        nb = read(STDIN_FILENO, &ticks, 2);
        if( nb == 0 ) exit(0);
        // This...
        // eventTime += (256.0*ticks)/(uCMHz*1E6);
        // ...is the true, wallclock-time increment.  But if we gnuplot that
        // then about 95% of our x-axis is periods of uart transmission.  So
        // instead we'll do this:
        eventTime += deltaT;

        // Draw a vertical line, to make the gaps between batches explicit.
        std::swap(vertlo,verthi);
        int const vertincr = 260;
        int vertdelta = (verthi-vertlo)/vertincr;
        for( int v=0;v<vertincr;++v)
        {
            printf("%10.7f %d\n",
                   eventTime,
                   vertlo + v*vertdelta);
        }
        printf("\n");
        eventTime += deltaT;
    }
}
