#include <stdio.h>

int main(int argc, char* argv[])
{
    int current_run = 0;
    int last_run = 0;

    while(1) {
        if (current_run == last_run) {
            printf("%d", 1);
            last_run = current_run + 1;
            current_run = 0;
        } else {
            printf("%d", 0);
        }
        current_run = current_run + 1;
    }
}
    