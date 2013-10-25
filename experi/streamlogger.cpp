//
// See $FWK/include/cpp/streamlogger.h.
//

#include <syslog.h>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <boost/shared_ptr.hpp>
using namespace std;

typedef unsigned uint32;

#define syslog(x) LoggerPtr(new Logger(x, __FUNCTION__, __FILE__, __LINE__))

class Logger;
typedef boost::shared_ptr<Logger> LoggerPtr;

class Logger {
private:
    uint32 const severity_;
    const char* function_;
    const char* file_;
    uint32 line_;
    static std::ostringstream& sstream();
public:
    Logger(uint32 severity, const char* function, const char* file, uint32 line)
      : severity_(severity), function_(function), file_(file), line_(line) {
        sstream().str("");
    }
    ~Logger() {
        printf("LOG_%d %s:%s:%d: %s\n", severity_, function_, file_, line_,
                sstream().str().c_str());
    }
    template<typename T> friend LoggerPtr operator<<(LoggerPtr, T const&);
};


std::ostringstream&
Logger::sstream()
{
    static std::ostringstream thing;
    return thing;
}


template<typename T> LoggerPtr operator<<(LoggerPtr l, T const& t)
{
    l->sstream() << t;
    return l;
}


int main()
{
    cout << "foo\n";
    syslog(LOG_INFO) << "pi is " << 3.14159 << "..., a transcendental number.";
    cout << "bar\n";
    syslog(LOG_WARNING) << "e, " << 2.7182 << "..., is also transcendental.";
}
