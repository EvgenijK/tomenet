#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(void)
{
    const char *delay = getenv("SV_PING_DELAY_MS");
    long milliseconds = delay ? strtol(delay, NULL, 10) : 300;
    if (milliseconds < 0 || milliseconds > 10000) return 2;
    struct timespec time = {milliseconds / 1000, milliseconds % 1000 * 1000000};
    nanosleep(&time, NULL);
    puts("64 bytes from fixture: time=42.2 ms");
    return 0;
}
