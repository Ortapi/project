#ifndef _myServer_H
#define __myServer_H_H

extern bool is_connected;

//extern SemaphoreHandle_t connect_sem;
// extern char *proj_data;
void start_mdns_service();
//void my_server_init();
void init_server();

esp_err_t send_ws_message(char *message);

#endif