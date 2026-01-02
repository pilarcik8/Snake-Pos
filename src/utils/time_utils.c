#include "time_utils.h"
#include <sys/time.h>
#include <unistd.h>

long time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L;
}

void sleep_ms(long ms) {
    // jednoduché: iba pre malé oneskorenia (napr. 50ms)
    usleep((useconds_t)(ms * 1000));
}

