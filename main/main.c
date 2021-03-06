#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_spiffs.h"
#include "led_strip.h"
#include "driver/rmt.h"
#include "ili9340.h"
#include "driver/i2c.h"
#include "lm75.h"
#include "floatstring.h"
//#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include <time.h>
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "ntp_time.h"
#include "esp_sntp.h"
#include "myServer.h"
#include <time.h>
#include "toggleLed.h"
#include "cJSON.h"
#include "mdns.h"
#include "esp_http_server.h"
#include "connect.h"
#include "resetBtn.h"
#include "sendmqtt.h"

#define TAG "FONT"
#define TAG_SYSTEM "SYSTEM TEST"
//SemaphoreHandle_t connect_sem;
SemaphoreHandle_t connectionSemaphore;
QueueHandle_t collectDataQueue;
QueueHandle_t collectDataQueue1;
QueueHandle_t setTemperatureQueue;

// extern TaskHandle_t MQTTtaskHandle;

bool is_connected = false;



void DisplayTask(void *params)
{
	esp_vfs_spiffs_conf_t spiffs_font_conf = {
      .base_path = "/font",
      .partition_label = "fonts",
      .max_files = 7,
      .format_if_mount_failed = true};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&spiffs_font_conf);
	
	//init font file from SPIFFS partition
	FontxFile fx32L[2];
	InitFontx(fx32L, "/font/LATIN32B.FNT", ""); // 16x32Dot Latinc
	TFT_t dev;

	//define unused pins as -1
	int GPIO_MISO = -1;
	int XPT_CS = -1;
	int XPT_IRQ = -1;
	int CONF_GPIO_BL = -1;
	int CONF_GPIO_RESET = -1;
	//spi init for desire pins
	spi_master_init(&dev,
					(int16_t)CONF_GPIO_MOSI,
					(int16_t)CONF_GPIO_SCLK,
					(int16_t)CONF_TFT_CS,
					(int16_t)CONF_GPIO_DC,
					(int16_t)CONF_GPIO_RESET,
					(int16_t)CONF_GPIO_BL,
					(int16_t)GPIO_MISO,
					(int16_t)XPT_CS,
					(int16_t)XPT_IRQ
					);

	lcdInit(&dev, (uint16_t)0x9341, CONF_TFT_WIDTH, CONF_TFT_HEIGHT, 0, 0);
	// lcdFillScreen(&dev, (uint16_t)0x6fa8dc);
	//// END OF HARDWARE CONFIG AND DEFINES /////

	/// START CONFIG WRITTEN TEXT ON SCREEN ///

	// FIRST FIXED POSITION HEADERS ////
	char *temperature_title = "Temperature:";
	char *time_title = "Time:";
	// char temp_conv[6];
	// float temp_raw;

	char *time_str_temp;

	lcdFillScreen(&dev, WHITE);
	lcdUnsetFontUnderLine(&dev);
	lcdSetFontFill(&dev, WHITE);
	printStrScreen(&dev, fx32L, temperature_title, 3);
	printStrScreen(&dev, fx32L, time_title, 7);

	char *temp_to_screen;
	
	
	while (1)
	{
		
		if (xQueueReceive(collectDataQueue, &temp_to_screen, portMAX_DELAY))
		{

			printStrScreen(&dev, fx32L, temp_to_screen, 5);
		}

		// temp_raw = readTemperature();
		// floatToString(temp_raw, temp_conv, 3);
		
		if (is_connected == true)
		{
			printStrScreen(&dev, fx32L, "       ", 9);
			time_str_temp = print_time_str();
			printStrScreen(&dev, fx32L, time_str_temp, 9);
		}
		else
		{
			time_str_temp = "waiting";
			printStrScreen(&dev, fx32L, time_str_temp, 9);
		}
		/// 5ms delay every screen write for temperature and NTP time
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void ledStripTask(void *params)
{

	uint32_t red = 0;
	uint32_t green = 0;
	uint32_t blue = 0;
	uint16_t hue = 0;
	uint16_t start_rgb = 0;

	rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_STRIP_GPIO, RMT_TX_CHANNEL);
	// set counter clock to 40MHz
	config.clk_div = 2;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

	// install ws2812 driver
	led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(LED_NUM, (led_strip_dev_t)config.channel);
	led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
	if (!strip)
	{
		ESP_LOGE(TAG, "install WS2812 driver failed");
	}
	// Clear LED strip (turn off all LEDs)
	ESP_ERROR_CHECK(strip->clear(strip, 100));

	ESP_LOGI(TAG, "LED Rainbow Chase Start");

	float temperature;
	while (1)
	{
		xQueueReceive(collectDataQueue1, &temperature, portMAX_DELAY);
		// temperature = 10;
		uint8_t decoded_temp = 100;
		for (int i = temperature; i > 0; i--)
		{
			decoded_temp -= 3;
		}

		// Build RGB values
		// hue = (decoded_temp) * 360 / LED_NUM;
		hue = decoded_temp;
		// printf("decoded_temp %d \n", (int)decoded_temp);
		led_strip_hsv2rgb(hue, 100, 20, &red, &green, &blue);
		for (int j = 0; j < LED_NUM; j++)
		{
			// Write RGB values to strip driver
			ESP_ERROR_CHECK(strip->set_pixel(strip, j, red, green, blue));
			vTaskDelay(50 / portTICK_PERIOD_MS);
			ESP_ERROR_CHECK(strip->refresh(strip, 100));
		}
		// Flush RGB values to LEDs
		vTaskDelay(1000 * 10 / portTICK_PERIOD_MS);
	}
}

// char *proj_data;
// static void makeJson(void *param)
// {
// 	char temp_conv[4];
// 	float temp_raw;
// 	char *time_str_temp;

// 	// printf("temperature: %f \n" ,temp_raw );
// 	// printf("temperature string: %s \n" ,temp_conv );

// 	xSemaphoreTake(connect_sem, portMAX_DELAY);
// 	is_connected = true;
// 	while (true)
// 	{
// 		time_str_temp = print_time_str();
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
// 		printf("ws message inside task:\n %s\n", proj_data);
// 		// send_ws_message(proj_data);
// 		cJSON_Delete(payload);
// 		//free(proj_data);

// 		// printf("ws message inside task:\n %s\n", message);
// 		// send_ws_message(message);
// 		// cJSON_Delete(payload);
// 		// free(message);
// 		// printf("makeJson test: %s \n", temp_conv )
// 		vTaskDelay(1000 * 10 / portTICK_PERIOD_MS);
// 	}
// }


void data_collect_task(void *param)
{
	i2c_lm75_init();
	char temp_conv[7];
	char *temp_buff = temp_conv;

	float temp_raw;

	while (1)
	{
	
	// temperature reading
	// convert float to string
	temp_raw = readTemperature();
	floatToString( temp_raw, temp_conv, 3);

	xQueueSend(collectDataQueue, &temp_buff, 0);
	xQueueSend(collectDataQueue1, &temp_raw, 0);

	// if(xQueueSend(collectDataQueue, &temp_buff, 0))
	// {

	// 	printf("added message to log queue %s  \n", temp_buff);
    // }
    // else
    // {
    //     printf("failed to add message to log queue\n");
        
    // }
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

}

void server_task(void *param)
{

	ESP_ERROR_CHECK(nvs_flash_init());
	init_led();
	wifi_init();
	wifi_connect_ap("esp32ap", "password");
	start_mdns_service();
	init_server();
	
	vTaskDelete(NULL);
}



void MQTT_task(void *param)
{
	// will be apply only when wifi connection ok
	xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
	float temperature;
	
	MQTT_app_start();
	while(true)
	{
		xQueueReceive(collectDataQueue1, &temperature, portMAX_DELAY);
		// ESP_LOGI(TAG_SYSTEM, "mqtt task test");
		mqtt_send_time();
		mqtt_send_temperature(temperature);

		
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}




void app_main(void)
{
	init_btn();
	connectionSemaphore = xSemaphoreCreateBinary();
	collectDataQueue = xQueueCreate(8, sizeof(char*));
	collectDataQueue1 = xQueueCreate(1, sizeof(float));
	setTemperatureQueue = xQueueCreate(1, sizeof(uint8_t));
	//connect_sem = xSemaphoreCreateBinary();
	xTaskCreate(data_collect_task, "data_collect_task", 1024 * 3, NULL, 0, NULL);

	xTaskCreate(DisplayTask, "ILI9340", 1024 * 3, NULL, 0, NULL);
	xTaskCreate(ledStripTask, "ws2812", 1024 * 2, NULL, 0, NULL);
	//xTaskCreate(makeJson, "makeJson", 1024 * 2, NULL, 2, NULL);
	xTaskCreatePinnedToCore(server_task, "server task", 1024 * 5, NULL, 5, NULL, 0);
	xTaskCreate(MQTT_task, "MQTT_task", 1024 * 2, NULL, 0, NULL);
	xTaskCreate(ac_cmd_task, "ac_cmd_task", 1024 * 2, NULL, 0, NULL);
}