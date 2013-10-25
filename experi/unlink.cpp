#include <unistd.h>

int main()
{
    int err = unlink("./temp");
    return err;
}
