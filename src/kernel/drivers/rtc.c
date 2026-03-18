#include "time.h"
#include "drivers/io.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static int get_update_in_progress_flag() {
    outb(CMOS_ADDR, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

void rtc_read_time(struct tm *time) {
    while (get_update_in_progress_flag());

    time->tm_sec  = get_rtc_register(0x00);
    time->tm_min  = get_rtc_register(0x02);
    time->tm_hour = get_rtc_register(0x04);
    time->tm_mday = get_rtc_register(0x07);
    time->tm_mon  = get_rtc_register(0x08) - 1; // 0-11
    time->tm_year = get_rtc_register(0x09);

    #define BCD2BIN(bcd) ((((bcd) & 0xF0) >> 4) * 10 + ((bcd) & 0x0F))
    
    time->tm_sec  = BCD2BIN(time->tm_sec);
    time->tm_min  = BCD2BIN(time->tm_min);
    time->tm_hour = BCD2BIN(time->tm_hour);
    time->tm_mday = BCD2BIN(time->tm_mday);
    time->tm_mon  = BCD2BIN(time->tm_mon);
    time->tm_year = BCD2BIN(time->tm_year) + 2000 - 1900;
}