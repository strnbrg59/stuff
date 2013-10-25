#include <boost/assign/list_of.hpp>

class Stage
{
public:
    Stage* next() const { return next_; }
    bool is_last() const { return !next_; }

private:
    Stage* next_;
};

/* Container and initializer */
class Stages
{
    static const Stage* download();
    static const Stage* install();
    static const Stage* rsp_image_download();
};


int main()
{
    Stages.init(

    Stage s = Stages::download();
    do_operation(s);
    s++;
