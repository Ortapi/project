#ifndef _NTP_TIME_H
#define _NTP_TIME_H

#include "connect.h"

extern SemaphoreHandle_t connect_sem;

void on_got_time(struct timeval *tv);
//void print_time_for_call_back(long time, const char *message);
void print_time(long time);
char *print_time_str();

#endif