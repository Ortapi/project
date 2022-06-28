#ifndef _sendmqtt_H
#define _sendmqtt_H

#define MQTT_SUBSCRIBE_LED      "led"
#define MQTT_SUBSCRIBE_SET      "set"
#define MQTT_TOPIC_TEMPERATURE  "temp"
#define MQTT_TOPIC_TIME         "time"
#define MQTT_TOPIC_MESSEGE      "messege"
#define MQTT_BROKER_URI     "mqtts://a79845304cad415dae16aacfc156b29c.s1.eu.hivemq.cloud:8883"
#define MQTT_CLIENT_ID      "control"
#define MQTT_USERNAME       "control"
#define MQTT_PASSWORD       "Or!15759363"

void mqtt_get_data(char *data_str, int data_size, char *topic, int topic_size);
void MQTT_app_start(void);

void MQTT_send_data(void);
void mqtt_send_temperature(float temperature);
void mqtt_send_time(void);
void ac_cmd_task(void *param);
void mqtt_publish_messege(char *msg);

#endif