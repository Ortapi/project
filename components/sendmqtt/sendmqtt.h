#ifndef _sendmqtt_H
#define _sendmqtt_H

#define MQTT_CLIENT_SUBSCRIBE   "led"
#define MQTT_TOPIC_TEMPERATURE  "temp"
#define MQTT_TOPIC_TIME         "time"
#define MQTT_BROKER_URI     "mqtts://a79845304cad415dae16aacfc156b29c.s1.eu.hivemq.cloud:8883"
#define MQTT_CLIENT_ID      "control"
#define MQTT_USERNAME       "control"
#define MQTT_PASSWORD       "Or!15759363"


void MQTT_app_start(void);

void MQTT_send_data(void);
void mqtt_send_temperature(void);
void mqtt_send_time(void);
void mqtt_check_bool(char *str, int size);

#endif