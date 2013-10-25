#include "c_auto_ptr.h"
#include "delfunc.h"
#include <string>

int main()
{
    c_auto_ptr<int>   icap(17);
    c_auto_ptr<float> fcap(3.14);
    c_auto_ptr<std::string> scap("howdy");
    MyClass mc;
    c_auto_ptr<MyClass> mccap(mc);
}
