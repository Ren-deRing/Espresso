#include "time.h"
#include "interrupt.h"

#include "drivers/lapic.h"

#include "serial.h"

static volatile uint64_t g_ticks = 0;
static time_t g_boot_epoch = 0;

void timer_handler(interrupt_frame_t *regs) {
    g_ticks++;
}

void time_init() {
    struct tm start_time;
    rtc_read_time(&start_time);
    
    g_boot_epoch = mktime(&start_time);

    lapic_timer_start(1, 0x20);
}

int clock_gettime(int clk_id, struct timespec *tp) {
    if (!tp) return -1;

    if (clk_id == CLOCK_REALTIME) {
        tp->tv_sec = g_boot_epoch + (g_ticks / 1000);
    } else {
        tp->tv_sec = (g_ticks / 1000);
    }
    tp->tv_nsec = (g_ticks % 1000) * 1000000;
    return 0;
}

void getTimeoutRelative(struct timespec *ts, long long millis, long long nanos) {
    if (!ts) return;

    long long total_nanos = (millis * 1000000LL) + nanos;

    ts->tv_sec = total_nanos / 1000000000LL;
    ts->tv_nsec = total_nanos % 1000000000LL;
}

void getTimeoutAbsolute(struct timespec *ts, long long millis, long long nanos) {
    if (!ts) return;

    struct timespec now;
    if (clock_gettime(0, &now) != 0) {
        ts->tv_sec = 0;
        ts->tv_nsec = 0;
        return;
    }

    long long total_nsec = now.tv_nsec + (millis * 1000000LL) + nanos;

    ts->tv_sec = now.tv_sec + (total_nsec / 1000000000LL);
    ts->tv_nsec = total_nsec % 1000000000LL;
}

static const int month_days[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

#define IS_LEAP(y) (((y) % 4 == 0 && (y) % 100 != 0) || ((y) % 400 == 0))

time_t mktime(struct tm *t) {
    time_t epoch = 0;
    int year = t->tm_year + 1900;
    int mon  = t->tm_mon;

    // 년도별 일수 합산
    for (int y = 1970; y < year; y++) {
        epoch += IS_LEAP(y) ? 366 : 365;
    }

    // 월별 일수 합산
    epoch += month_days[mon];
    
    // 만약 현재가 3월 이후이고, 올해가 윤년이면 하루 더함
    if (mon > 1 && IS_LEAP(year)) {
        epoch += 1;
    }

    // 일 합산
    epoch += t->tm_mday - 1;

    epoch = epoch * 86400LL; // 하루
    epoch += t->tm_hour * 3600LL;
    epoch += t->tm_min * 60LL;
    epoch += t->tm_sec;

    return epoch;
}