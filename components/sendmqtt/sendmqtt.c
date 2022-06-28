#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
extern QueueHandle_t collectDataQueue1;
extern QueueHandle_t setTemperatureQueue;
// TaskHandle_t MQTTtaskHandle;


static bool ac_on_off = false;

esp_mqtt_client_handle_t client = NULL;
extern SemaphoreHandle_t connectionSemaphore;

//HIVEMQ MQTT cloude server cerificate
extern const uint8_t isrgrootx1[] asm("_binary_isrgrootx1_pem_start");


// static const int AC_SYSTEM_ON = BIT0;
// static const int AC_SYSTEM_OFF = BIT1;


void mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  
  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    esp_mqtt_client_subscribe(client, MQTT_SUBSCRIBE_LED, 2);
    esp_mqtt_client_subscribe(client, MQTT_SUBSCRIBE_SET, 2);
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
    // mqtt_get_data(event->data, event->data_len ,event->topic ,event->topic_len);
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
  esp_mqtt_client_publish(client, MQTT_TOPIC_TIME, time_str_temp, strlen(time_str_temp), 2, false);
}

void mqtt_send_temperature(float temperature){
  char temp_conv[4];

	// convert float to string
	floatToString(temperature, temp_conv, 3);
  esp_mqtt_client_publish(client, MQTT_TOPIC_TEMPERATURE, temp_conv, 4, 2, false);
}

void mqtt_publish_messege(char *msg){
  
  esp_mqtt_client_publish(client, MQTT_TOPIC_MESSEGE, msg, strlen(msg), 2, false);
}







void mqtt_get_data(char *data_str, int data_size, char *topic, int topic_size)
{
  uint8_t set_temperature;
  uint8_t uint_data;

  char topic_buffer[topic_size];
  char data_buffer[data_size];
  int i = 0;

  for(i = 0; i <= topic_size; i++) {
    if(i == topic_size) topic_buffer[i] = '\0';
      else topic_buffer[i] = topic[i];
    }

    for(i = 0; i <= data_size; i++) {
        if(i == data_size) data_buffer[i] = '\0';
        else data_buffer[i] = data_str[i];
    }

    printf("mqtt_get_data TOPIC= %s \r\n", topic_buffer);
    printf("mqtt_get_data DATA= %s \r\n", data_buffer);
  


  if (strcmp(topic_buffer, "set") == 0)
  {
    set_temperature = (uint8_t)atoi(data_buffer);
    printf("mqtt set temp: %d \n", set_temperature);
    mqtt_publish_messege("TEMPERATURE IS SET TO:");
    esp_mqtt_client_publish(client, MQTT_TOPIC_MESSEGE, data_buffer, strlen(data_buffer), 2, false);
    xQueueSend(setTemperatureQueue, &set_temperature, 0);
  }

  if (strcmp(topic_buffer, "led") == 0)
  {
      uint_data = (uint8_t)atoi(data_buffer);
      // char checkStr[] = "led topic check";
      // esp_mqtt_client_publish(client, MQTT_TOPIC_MESSEGE, checkStr, strlen(checkStr), 2, false);
      if (uint_data == 1)
      {
        ac_on_off = true;
        // printf("ac_on_off true \n");
        mqtt_publish_messege("ON!");
        // xTaskNotify(MQTTtaskHandle, AC_SYSTEM_ON, eSetValueWithOverwrite);
      }
      if (uint_data == 0)
      {
        ac_on_off = false;
        // printf("ac_on_off false \n");
        mqtt_publish_messege("OFF!");
        // xTaskNotify(MQTTtaskHandle, AC_SYSTEM_OFF, eSetValueWithOverwrite);
      }
  }
}

void ac_cmd_task(void *param)
{
// xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
  
  // bool high_hysteresis_checkd = false;
  // bool low_hysteresis_checkd = false;
  // bool off_state_checked = true;

  // default temperature
  uint8_t set_temperature = 25;
  float temperature_sensor;

    while (1)
  {

    // get the wanted temperature from user by queue
    if (xQueueReceive(setTemperatureQueue,&set_temperature, 0))
    {
      printf("QueueReceive set_temperature %d  \n", set_temperature);
    }

    if (ac_on_off == true)
    {
      // printf("inside true ac_on_off if \n");
      
      if (xQueueReceive(collectDataQueue1,&temperature_sensor, 0))
      {
        // printf("QueueReceive temperature_sensor %d  \n", (uint8_t)temperature_sensor);
        
        if ((uint8_t)temperature_sensor > set_temperature + 1)
        {
          // printf("hysteresis is high -- relay on \n");
          toggle_led(true);
        }

        if ((uint8_t)temperature_sensor < set_temperature - 1)
        {
          // printf("hysteresis is low -- relay off \n");
          toggle_led(false);
        }
      }
    }

    if (ac_on_off == false)
    {
      toggle_led(false);
      // printf("inside false ac_on_off if \n");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  
}



