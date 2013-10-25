#include <syslog.h>

int main()
{
    syslog(LOG_CRIT, "Hello %s.\n", "world");
    return 0;
}
