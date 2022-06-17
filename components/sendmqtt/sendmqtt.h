#ifndef _sendmqtt_H
#define _sendmqtt_H

#define MQTT_CLIENT_SUBSCRIBE    "led"
#define MQTT_CLIENT_PUBLISH      "temp"
#define MQTT_BROKER_URI     "mqtts://a79845304cad415dae16aacfc156b29c.s1.eu.hivemq.cloud:8883"
#define MQTT_CLIENT_ID      "control"
#define MQTT_USERNAME       "control"
#define MQTT_PASSWORD       "Or!15759363"


// void mqtt_init();
// void MQTTLogic(char *sensorReading);
// void OnConnected();
void MQTT_app_start(void);
// void send_mqtt_cmd(void *param);

#endif