#pragma once

#include <stdint.h>

typedef long time_t;
typedef uint64_t clock_t;

struct tm {
    uint64_t tm_sec;   // 0-59
    uint64_t tm_min;   // 0-59
    uint64_t tm_hour;  // 0-23
    uint64_t tm_mday;  // 1-31
    uint64_t tm_mon;   // 0-11
    uint64_t tm_year;  // 1900부터
};

struct timespec {
    time_t tv_sec;      // 초
    long   tv_nsec;     // 나노초
};

struct timeval {
    time_t tv_sec;      // 초
    time_t tv_usec;     // 마이크로초
};

#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          1

void time_init();

int clock_gettime(int clk_id, struct timespec *tp);
clock_t clock(void);
int gettimeofday(struct timeval *tv, void *tz);

void rtc_read_time(struct tm *time);
time_t mktime(struct tm *t);

void getTimeoutAbsolute(struct timespec *ts, long long millis, long long nanos);
void getTimeoutRelative(struct timespec *ts, long long millis, long long nanos);