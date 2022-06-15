#include <stdio.h>
#include "connect.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mdns.h"
#include "toggleLed.h"
#include "cJSON.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include "ntp_time.h"
#include "esp_sntp.h"
#include "myServer.h"
#include "floatstring.h"
#include "lm75.h"


// #define WIFI_STA_SSID "ortapiero_2.4"
// #define WIFI_STA_PASS "0522255504"

// #define WIFI_STA_SSID "PET"
// #define WIFI_STA_PASS "048297100"


static const char *TAG = "SERVER";
#define AMX_APs 4

static httpd_handle_t server = NULL;

static esp_err_t on_default_url(httpd_req_t *req)
{
  ESP_LOGI(TAG, "URL: %s", req->uri);

  esp_vfs_spiffs_conf_t esp_vfs_spiffs_conf = {
      .base_path = "/site/build",
      .partition_label = "web",
      .max_files = 16,
      .format_if_mount_failed = true};
  esp_vfs_spiffs_register(&esp_vfs_spiffs_conf);




  char path[600];
  if (strcmp(req->uri, "/") == 0 )
  {
    strcpy(path, "/site/build/index.html");
  }
  else
  {
    sprintf(path, "/site/build%s", req->uri);
  }


  char *ext = strrchr(path, '.');
  if (strcmp(ext, ".local") == 0)
  {
    httpd_resp_set_type(req, "text/html");
  }
  if (strcmp(ext, ".css") == 0)
  {
    httpd_resp_set_type(req, "text/css");
  }
  if (strcmp(ext, ".js") == 0)
  {
    httpd_resp_set_type(req, "text/javascript");
  }
  if (strcmp(ext, ".png") == 0)
  {
    httpd_resp_set_type(req, "image/png");
  }
  if (strcmp(ext, ".json") == 0)
  {
    httpd_resp_set_type(req, "application/json");
  }

  

  FILE *file = fopen(path, "r");
  if (file == NULL)
  {
    httpd_resp_send_404(req);
    esp_vfs_spiffs_unregister("web");
    return ESP_OK;
  }

  char lineRead[256];
  while (fgets(lineRead, sizeof(lineRead), file))
  {
    httpd_resp_sendstr_chunk(req, lineRead);
  }
  httpd_resp_sendstr_chunk(req, NULL);
  
  esp_vfs_spiffs_unregister("web");
  return ESP_OK;
}

static esp_err_t on_toggle_led_url(httpd_req_t *req)
{
  char buffer[100];
  memset(&buffer, 0, sizeof(buffer));
  httpd_req_recv(req, buffer, req->content_len);
  cJSON *payload = cJSON_Parse(buffer);
  cJSON *is_on_json = cJSON_GetObjectItem(payload, "is_on");
  bool is_on = cJSON_IsTrue(is_on_json);
  cJSON_Delete(payload);
  toggle_led(is_on);
  httpd_resp_set_status(req, "204 NO CONTENT");
  httpd_resp_send(req, NULL, 0);
  return ESP_OK;
}


static esp_err_t on_proj_data_url(httpd_req_t *req)
{
  char *proj_data;
  char temp_conv[4];
  float temp_raw;
  char *time_str_temp;

  cJSON *json_array = cJSON_CreateArray();
  cJSON *payload = cJSON_CreateObject();

  if (is_connected == true)
  {
    time_str_temp = print_time_str();
  }
  else
  {
    time_str_temp = "waiting for connection";
  }

    temp_raw = readTemperature();
    // convert float to string
    ftoa(temp_raw, temp_conv, 3);

  cJSON_AddStringToObject(payload, "temperature_str", temp_conv);
  cJSON_AddStringToObject(payload, "time_str", time_str_temp);
  cJSON_AddItemToArray(json_array, payload);
  // char *message = cJSON_Print(payload);
  proj_data = cJSON_Print(payload);
  // send_ws_message(proj_data);
  cJSON_Delete(payload);


  ESP_LOGI(TAG, "on show data url: %s" ,proj_data );
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, proj_data);
  free(proj_data);
  return ESP_OK;
}



////////////////////////////////////////////////
/////////////// START SCAN FOR WIFI AP's///////
//////////////////////////////////////////////
// static esp_err_t on_get_ap_url(httpd_req_t *req)
// {

//   wifi_scan_config_t wifi_scan_config = {
//       .bssid = 0,
//       .ssid = 0,
//       .channel = 0,
//       .show_hidden = true};

//   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

//   ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, false));  



//   wifi_ap_record_t wifi_ap_record[AMX_APs];
//   uint16_t ap_count = AMX_APs;
//   esp_wifi_scan_get_ap_records(&ap_count, wifi_ap_record);
//   cJSON *wifi_scan_json = cJSON_CreateArray();
//   for (size_t i = 0; i < ap_count; i++)
//   {
//     cJSON *element = cJSON_CreateObject();
//     cJSON_AddStringToObject(element, "ssid", (char *)wifi_ap_record[i].ssid);
//     cJSON_AddNumberToObject(element, "rssi", wifi_ap_record[i].rssi);
//     cJSON_AddItemToArray(wifi_scan_json, element);
//   }
//   char *json_str = cJSON_Print(wifi_scan_json);
//   httpd_resp_set_type(req, "application/json");
//   httpd_resp_sendstr(req, json_str);
  
//   cJSON_Delete(wifi_scan_json);
//   free(json_str);
  
//   return ESP_OK;
// }

/********************AP TO STA *******************/

typedef struct ap_config_t
{
  char ssid[32];
  char password[32];
} ap_config_t;

void connect_to_ap(void *params)
{
  vTaskDelay(pdMS_TO_TICKS(500));
  ap_config_t *ap_config = (ap_config_t *)params;
  wifi_disconnect();
  wifi_destroy_netif();
  

  esp_err_t err = wifi_connect_sta(ap_config->ssid, ap_config->password, 10000);

  if (err == ESP_OK)
  {
      ESP_LOGI(TAG, "CONNECTED TO AP");
      
      is_connected = true;
      sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
      sntp_setservername(0, "pool.ntp.org");
      sntp_init();
      sntp_set_time_sync_notification_cb(on_got_time);
      print_time_str();
  }
  else
  {
    is_connected = false;
    ESP_LOGE(TAG, "Failed to connect to AP");
    wifi_connect_ap("esp32ap", "password");
  }
  vTaskDelete(NULL);
}

static esp_err_t on_ap_to_sta(httpd_req_t *req)
{
  static ap_config_t ap_config;

  char buffer[100];
  memset(&buffer, 0, sizeof(buffer));
  httpd_req_recv(req, buffer, req->content_len);
  cJSON *payload = cJSON_Parse(buffer);
  strcpy(ap_config.ssid, cJSON_GetObjectItem(payload, "ssid")->valuestring);
  strcpy(ap_config.password, cJSON_GetObjectItem(payload, "password")->valuestring);
  cJSON_Delete(payload);

  httpd_resp_set_status(req, "204 NO CONTENT");
  httpd_resp_send(req, NULL, 0);

  xTaskCreate(connect_to_ap, "connect_to_ap", 1024 * 5, &ap_config, 1, NULL);

  return ESP_OK;
}

/********************Web Socket*******************/

// #define WS_MAX_SIZE 1024
// static int client_session_id;

// esp_err_t send_ws_message(char *message)
// {
//   if (!client_session_id)
//   {
//     ESP_LOGE(TAG, "no client_session_id");
//     return -1;
//   }
//   httpd_ws_frame_t ws_message = {
//       .final = true,
//       .fragmented = false,
//       .len = strlen(message),
//       .payload = (uint8_t *)message,
//       .type = HTTPD_WS_TYPE_TEXT};
//   return httpd_ws_send_frame_async(server, client_session_id, &ws_message);
// }

// static esp_err_t on_web_socket_url(httpd_req_t *req)
// {
//   client_session_id = httpd_req_to_sockfd(req);
//   if (req->method == HTTP_GET)
//     return ESP_OK;

//   httpd_ws_frame_t ws_pkt;
//   memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
//   ws_pkt.type = HTTPD_WS_TYPE_TEXT;
//   ws_pkt.payload = malloc(WS_MAX_SIZE);
//   httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE);
//   printf("ws payload: %.*s\n", ws_pkt.len, ws_pkt.payload);
//   free(ws_pkt.payload);

//   char *response = "connected OK";
//   httpd_ws_frame_t ws_responce = {
//       .final = true,
//       .fragmented = false,
//       .type = HTTPD_WS_TYPE_TEXT,
//       .payload = (uint8_t *)response,
//       .len = strlen(response)};
//   return httpd_ws_send_frame(req, &ws_responce);
// }



/*******************************************/

void init_server()
{

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_ERROR_CHECK(httpd_start(&server, &config));

  // httpd_uri_t get_ap_list_url = {
  //     .uri = "/api/get-ap-list",
  //     .method = HTTP_GET,
  //     .handler = on_get_ap_url};
  // httpd_register_uri_handler(server, &get_ap_list_url);

  httpd_uri_t proj_data_url = {
      .uri = "/api/proj-data",
      .method = HTTP_GET,
      .handler = on_proj_data_url};
  httpd_register_uri_handler(server, &proj_data_url);

  httpd_uri_t toggle_led_url = {
      .uri = "/api/toggle-led",
      .method = HTTP_POST,
      .handler = on_toggle_led_url};
  httpd_register_uri_handler(server, &toggle_led_url);

  httpd_uri_t ap_to_sta_url = {
      .uri = "/api/ap-sta",
      .method = HTTP_POST,
      .handler = on_ap_to_sta};
  httpd_register_uri_handler(server, &ap_to_sta_url);

  // httpd_uri_t web_socket_url = {
  //     .uri = "/ws",
  //     .method = HTTP_GET,
  //     .handler = on_web_socket_url,
  //     .is_websocket = true};
  // httpd_register_uri_handler(server, &web_socket_url);

  httpd_uri_t default_url = {
      .uri = "/*",
      .method = HTTP_GET,
      .handler = on_default_url};
  httpd_register_uri_handler(server, &default_url);
  
}

void start_mdns_service()
{
  mdns_init();
  mdns_hostname_set("my-esp32");
  mdns_instance_name_set("or tapiero project");
}


// void my_server_init(void)
// {

//   init_led();

//   //init_btn();
//   wifi_init();
//   //ESP_ERROR_CHECK(wifi_connect_sta(WIFI_STA_SSID, WIFI_STA_PASS, 10000));
//   wifi_connect_ap("esp32ap", "password");
  
//   start_mdns_service();
//   init_server();
// }
