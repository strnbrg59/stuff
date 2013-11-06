//
// Simple serial port reader/writer.
// Based on http://www.easysw.com/~mike/serial/serial.html.
// Works well with AVR sending messages with the following code:
/*
void put_char(char c) {
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}

void send_message(char * msg) {
    char* c = msg;
    while(*c) {
        if (*c == '\n') put_char('\r');
        else            put_char(*c);
        c++;
    }
    put_char(0);
}
*/
//

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <string>
#include <algorithm>
#include <map>

speed_t baudCode(long int baud)
{
    // Got these from "man cfsetispeed".
    typedef std::map<long int, speed_t> CodeMap;
    CodeMap codes;
    codes[300] = B300;
    codes[600] = B600;
    codes[1200] = B1200;
    codes[1800] = B1800;
    codes[2400] = B2400;
    codes[4800] = B4800;
    codes[9600] = B9600;
    codes[19200] = B19200;
    codes[38400] = B38400;
    codes[57600] = B57600;
    codes[115200] = B115200;
    codes[230400] = B230400;
    
    CodeMap::const_iterator i = codes.find(baud);
    if( i == codes.end() )
    {
        fprintf(stderr, "Invalid baud: %ld\n", baud);
        exit(4);
    }
    return i->second;
}


void
open_port( int& fd, int baud )
{
  char* portname = getenv("MYTTY");
  assert( portname );
  fd = open( portname , O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    perror("open_port: Unable to open serial port - ");
    exit(2);
  } else
  {
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baudCode(baud));
    cfsetospeed(&options, baudCode(baud));
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    fcntl(fd, F_SETFL, 0);  // read(2) will be blocking.
    tcsetattr(fd, TCSANOW, &options);
    
    assert(!(options.c_iflag & IXON));
    assert(!(options.c_iflag & INPCK));
  }
}

void doRead(bool debug, int fd, bool binaryMode)
{
    int const buflen(20);
    char buf[buflen];

    while(1)
    {
        int bytes;
        if( debug )
        {
            assert(!binaryMode);

            //ioctl(fd, FIONREAD, &bytes);
            //printf("%d bytes available...\n", bytes);
            bytes = read( fd, buf, buflen );
            printf("read(2) returned %d bytes: ", bytes);
            for( int i=0;i<bytes;++i )
            {
                if( buf[i] > 31 )
                {
                    printf("char=%c (0x%X), ", buf[i], buf[i]);
                } else
                {
                    printf("(0x%X), ", buf[i]);
                }
            }
            printf("\n");
        } else
        {
            memset(buf, 0, buflen);
            bytes = read( fd, buf, buflen );

            if( binaryMode )
            {
                write(STDOUT_FILENO, buf, bytes);
            } else
            {   // Taking care to identify zero-terminated ascii strings...
                int pos = 0;
                while( pos < bytes )
                {
                    printf(buf+pos);
                    pos += std::max(strlen(buf+pos),size_t(1));
                }
            }
        }
    }
}

void doWrite(int fd)
{
    // Write to the port.  Read a string from the keyboard, but replace its
    // terminal '\n' with a '\0'.
    // That's what my AVR UART::gets() expects.
    char buf[80];
    while(1)
    {
        printf("> ");
        fgets(buf, 80, stdin);
        buf[strlen(buf)-1] = 0;
        printf("Writing |%s|\n", buf);
        int nb = write( fd, buf, strlen(buf)+1 );
        if( 0 > nb )
        {
            perror("");
            exit(1);
        }
        printf("Wrote %d bytes: ", nb);
        for(int i=0;i<nb;++i ) printf("%X ", buf[i]);
        printf("\n");
    }
}


void saveOptArg( std::string& batchBytes )
{
    batchBytes = std::string(optarg);
    batchBytes += '\0';
    // Needed that trailing 0x0 for AVR UART::gets().
}


/** We need this, because for the output of this process to be available
 *  to the Python subprocess.Popen.stdout object, this process needs to
 *  first exit with return code 0.  Or so it seems to me based on experiments.
*/
void sigUSR1handler(int sn)
{
    if( sn == SIGUSR1 )
    {
        exit(0);
    }
}


/** Write exactly what's in the string, UNLESS the string looks like a hex
 *  byte in 0x?? form, in which case write just that byte.
*/
void writeBatchBytes(std::string batchBytes, int fd)
{
    char const hexes[] = "0123456789abcdefABCDEF";
    if( (   !strncmp(batchBytes.c_str(),"0x",2)
         || !strncmp(batchBytes.c_str(),"0X",2) )
    &&  (strlen(batchBytes.c_str()) == 4)
    &&  (strchr(hexes,batchBytes[2]))
    &&  (strchr(hexes,batchBytes[3])) )
    {
        char h;
        sscanf(batchBytes.c_str(), "%x", &h);
        write( fd, &h, 1 );
    } else
    {
        write( fd, batchBytes.c_str(), batchBytes.size() );
    }
}


int main(int argc, char* argv[])
{
    struct sigaction action;
    action.sa_handler = sigUSR1handler;
    sigaction(SIGUSR1, &action, 0);

    int c;
    bool readMode=false;
    bool debugReadMode=false;
    bool writeMode=false;
    std::string batchBytes;
    bool binaryMode=false;
    int baud=0;
    while ( ( c = getopt( argc, argv, "rRwx:bs:" ) )  != EOF )
    {
        switch ( c )
        {
            case 'r': readMode = true; break;
            case 'R': debugReadMode = true; break;
            case 'w': writeMode = true; break;
            case 'x': saveOptArg(batchBytes); break;
            case 's': baud = atoi(optarg); break;
            case 'b': binaryMode = true; break;
            default: assert(0);
        }
    }
    
    if( baud==0 )
    {
        fprintf(stderr, "Error: must set baud rate!\n");
        exit(1);
    }

    int fd;
    open_port(fd, baud);

    if( readMode )
    {
        doRead(false, fd, binaryMode);
    } else if( debugReadMode )
    {
        doRead(true, fd, binaryMode);
    } else if( writeMode )
    {
        doWrite(fd);
    } else if(!batchBytes.empty())
    {
        writeBatchBytes(batchBytes, fd);
    } else
    { 
        fprintf(stderr, "Must specify -r, -R or -w (in addition to -b).\n");
        exit(3);
    }
}
