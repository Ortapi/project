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

#define TAG "MQTT"

TaskHandle_t MQTTtaskHandle;
extern SemaphoreHandle_t connectionSemaphore;

//HIVEMQ MQTT cloude server cerificate
extern const uint8_t isrgrootx1[] asm("_binary_isrgrootx1_pem_start");


static const int WIFI_CONNECTED = BIT0;
static const int MQTT_CONNECTED = BIT1;
static const int MQTT_PUBLISHED = BIT2;

// mqtt handler call back function
// uses as a state machine 
void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    xTaskNotify(MQTTtaskHandle, MQTT_CONNECTED, eSetValueWithOverwrite);
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

void MQTTLogic(char *sensorReading)
{
  uint32_t command = 0;
  esp_mqtt_client_config_t mqttConfig = {
      .uri = MQTT_BROKER_URI,
      .client_id = MQTT_CLIENT_ID,
      .username = MQTT_USERNAME,
      .password = MQTT_PASSWORD,
      .cert_pem = (char *)isrgrootx1
      };
  esp_mqtt_client_handle_t client = NULL;

  while (true)
  {
    xTaskNotifyWait(0, 0, &command, portMAX_DELAY);
    switch (command)
    {
    case WIFI_CONNECTED:
      client = esp_mqtt_client_init(&mqttConfig);
      esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
      esp_mqtt_client_start(client);
      break;
    case MQTT_CONNECTED:
      esp_mqtt_client_subscribe(client, CLIENT_SUBSCRIBE, 2);
      char data[50];
      sprintf(data, "%s", sensorReading);
      printf("sending data: %s", sensorReading);
      esp_mqtt_client_publish(client, CLIENT_PUBLISH, data, strlen(data), 2, false);
      break;
    case MQTT_PUBLISHED:
// ` 
//       esp_mqtt_client_stop(client);
//       esp_mqtt_client_destroy(client);
//       esp_wifi_stop();
      return;
    default:
      break;
    }
  }
}