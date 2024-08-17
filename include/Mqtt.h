#ifndef MQTT_H_
#define MQTT_H_

#include "main.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <cJSON.h>
#include "Wifi_Config.h"

// 配置MQTT服务器
#define SERVERIP "192.168.50.111"
#define PORT 1883
#define SERVER_USER "esp32"
#define USER_PASSWD "123456"
#define SUB_TOPIC "/python/esp32_mqtt"
#define CLIENTID "esp32s3"

typedef struct
{
    bool status;
    uint8_t retry_cnt;
    uint8_t mqtt_maunual_flag; // mqtt手动操作断开or连接
    uint8_t mqtt_connect_flag; // 由中断控制每100ms检查一次
} MQTT;

extern MQTT mqtt;

void mqtt_connect(void);
void mqtt_loop(void);
void mqtt_disconnect(void);
#endif