#define SOCKETFILENAME "shiq?i"
#define SERVSOCKFILENAME "server-shiq?i"
#define SERVACTIVITYFILE "server-activity.dat"
#define MAXTRIES 10 // times server_ready() will try to see if server is ready.

int server_avail();
void server_ready( int handle );
void server_goodbye( int handle );
long microtime();

enum { server_down=-1, server_up=1 };

