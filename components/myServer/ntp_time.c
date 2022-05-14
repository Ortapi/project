#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "connect.h"
#include "esp_log.h"
#include "ntp_time.h"

#define TAG "NTP_TIME"



char *print_time_str()
{

  time_t now;
  static char strftime_buf[9];
  struct tm timeinfo;

    time(&now);
    setenv("TZ", "IST-2IDT,M3.4.4/26,M10.5.002:00:00,M3.5.0/03:00:00", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);

    strftime_buf[9] = '\0';

  return strftime_buf;
}

void print_time(long time)
{
  setenv("TZ", "IST-2IDT,M3.4.4/26,M10.5.002:00:00,M3.5.0/03:00:00", 1);
  tzset();
  struct tm *timeinfo = localtime(&time);

  char buffer[50];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  printf("time: %s\n", buffer);
}





void on_got_time(struct timeval *tv)
{
  print_time(tv->tv_sec);
}

// void app_main(void)
// {
//   time_t now = 0;
//   time(&now);
//   print_time(now, "Beginning of application");

//   nvs_flash_init();
//   tcpip_adapter_init();
//   esp_event_loop_create_default();
//   example_connect();

//   sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
//   sntp_setservername(0, "pool.ntp.org");
//   sntp_init();
//   sntp_set_time_sync_notification_cb(on_got_time);
// }