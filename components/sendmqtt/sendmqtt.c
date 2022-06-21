#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "connect.h"
#include "mqtt_client.h"
#include "sendmqtt.h"
#include "esp_tls.h"
#include "toggleLed.h"
#include "floatstring.h"
#include "lm75.h"
#include "ntp_time.h"

#define TAG "MQTT"

TaskHandle_t MQTTtaskHandle;

esp_mqtt_client_handle_t client = NULL;
// extern SemaphoreHandle_t connectionSemaphore;

//HIVEMQ MQTT cloude server cerificate
extern const uint8_t isrgrootx1[] asm("_binary_isrgrootx1_pem_start");


// static const int WIFI_CONNECTED = BIT0;
// static const int MQTT_CONNECTED = BIT1;
// static const int MQTT_PUBLISHED = BIT2;

// static const int MQTT_SEND          = BIT3;


void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    esp_mqtt_client_subscribe(client, MQTT_CLIENT_SUBSCRIBE, 2);
    //xTaskNotify(MQTTtaskHandle, MQTT_CONNECTED, eSetValueWithOverwrite);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    //xTaskNotify(MQTTtaskHandle, MQTT_PUBLISHED, eSetValueWithOverwrite);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    // char *data_mqtt;
    // data_mqtt = event->data;
    mqtt_check_bool(event->data, event->data_len);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  mqtt_event_handler_cb(event_data);
}

void MQTT_app_start(void)
{
  esp_mqtt_client_config_t mqttConfig = {
    .uri = MQTT_BROKER_URI,
    .client_id = MQTT_CLIENT_ID,
    .username = MQTT_USERNAME,
    .password = MQTT_PASSWORD,
    .cert_pem = (char *)isrgrootx1
    };
  client = esp_mqtt_client_init(&mqttConfig);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  esp_mqtt_client_start(client);
}

// void MQTT_send_data(void)
// {
//   char *proj_data;
//   char temp_conv[4];
// 	float temp_raw;
// 	char *time_str_temp;
//   time_str_temp = print_time_str();
// 		temp_raw = readTemperature();
// 		////convert float to string
// 		floatToString(temp_raw, temp_conv, 3);
// 		cJSON *json_array = cJSON_CreateArray();
// 		cJSON *payload = cJSON_CreateObject();
// 		// char *time_str_temp = print_time_str();
// 		cJSON_AddStringToObject(payload, "temperature_str", temp_conv);
// 		cJSON_AddStringToObject(payload, "time_str", time_str_temp);
// 		cJSON_AddItemToArray(json_array, payload);
// 		//char *message = cJSON_Print(payload);
// 		proj_data = cJSON_Print(payload);
// 		// printf("MQTT_send_data :\n %s\n", proj_data);
// 		// send_ws_message(proj_data);
//     esp_mqtt_client_publish(client, MQTT_CLIENT_PUBLISH, proj_data, strlen(proj_data), 0, false);
// 		cJSON_Delete(payload);
// 		//free(proj_data);
// }


void mqtt_send_time(void){
  char *time_str_temp;
  time_str_temp = print_time_str();
  esp_mqtt_client_publish(client, MQTT_TOPIC_TIME, time_str_temp, strlen(time_str_temp), 0, false);
}

void mqtt_send_temperature(void){
  char temp_conv[4];
	float temp_raw;
  temp_raw = readTemperature();
	// convert float to string
	floatToString(temp_raw, temp_conv, 3);
  esp_mqtt_client_publish(client, MQTT_TOPIC_TEMPERATURE, temp_conv, 4, 0, false);
}

void mqtt_check_bool(char *str, int size){

    uint8_t i = 0;
    // char str_arr = str[0];
    char str_arr[size];
    // memset(str_arr, size, sizeof(char));

    // int count = 0;
    if (size == 1)
    {
        for (i = 0; i < size; i++)
        {
            // count++;
            str_arr[i] = str[i];
            // printf("str = %c \n" , str_arr[i]);
            // printf("count = %d \n" , count);
        }

        if (str_arr[0] == '1')
        {
          toggle_led(true);
          ESP_LOGI(TAG, "MQTT relay on");
        }
        else if (str_arr[0] == '0')
        {
            toggle_led(false);
            ESP_LOGI(TAG, "MQTT relay off");
        }
        else{
            ESP_LOGI(TAG, "MQTT relay not bool received");
        }
    }
    if (size > 1)
    {
        ESP_LOGI(TAG, "MQTT relay bool payload > 1");
    }
    

}