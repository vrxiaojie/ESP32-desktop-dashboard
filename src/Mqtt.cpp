/********************************** MQTT配置 **********************************/
#include "Mqtt.h"

WiFiClient espclient;
PubSubClient client;

MQTT mqtt = {false, 0, 0, 0};

// MQTT回调函数
void MQTT_callback(char *topic, byte *payload, unsigned int length)
{
    char str1[200];
    // 将uint8_t转char类型
    for (int i = 0; i < length; i++)
    {
        str1[i] = payload[i];
    }
    // 解析JSON数据
    cJSON *cjson = cJSON_Parse(str1);
    // 将数据逐个放置进数据数组
    ui.PC_monitor[0] = std::stof(cJSON_GetObjectItem(cjson, "CPU_usage")->valuestring);
    ui.PC_monitor[1] = std::stof(cJSON_GetObjectItem(cjson, "CPU_Frequency")->valuestring);
    ui.PC_monitor[2] = std::stof(cJSON_GetObjectItem(cjson, "RAM_usage")->valuestring);
    ui.PC_monitor[3] = std::stof(cJSON_GetObjectItem(cjson, "GPU_temp")->valuestring);
    ui.PC_monitor[4] = std::stof(cJSON_GetObjectItem(cjson, "GPU_memory")->valuestring);
}
// 连接MQTT
void mqtt_connect()
{
    client.setClient(espclient);
    client.setServer(SERVERIP, PORT);
    client.setCallback(MQTT_callback);
    if (WiFi.status() == WL_CONNECTED && mqtt.mqtt_connect_flag == 1 
    && mqtt.mqtt_maunual_flag == 0 && mqtt.retry_cnt < 3) // 由定时器控制，若意外断开(非手动) 则重连
    {
        Serial.println("Connecting to MQTT...");
        if (client.connect(CLIENTID, SERVER_USER, USER_PASSWD))
        {
            Serial.println("MQTT Connected");
            client.subscribe(SUB_TOPIC);
            mqtt.mqtt_connect_flag = 0;
            mqtt.retry_cnt = 0;
        }
        else
        {
            Serial.println("Login Failed");
            Serial.println(client.state());
            mqtt.retry_cnt++;
        }
    }
    if (WiFi.status() == WL_CONNECTED && mqtt.mqtt_connect_flag == 1 
    && mqtt.mqtt_maunual_flag == 1 && mqtt.retry_cnt < 3) // 若为手动断开的，则不会自动重连，只有在手动连接时执行该代码块
    {
        Serial.println("Manually Connect to MQTT...");
        if (client.connect(CLIENTID, SERVER_USER, USER_PASSWD))
        {
            Serial.println("MQTT Connected");
            client.subscribe(SUB_TOPIC);
            mqtt.retry_cnt = 0;
        }
        else
        {
            mqtt.retry_cnt++;
            Serial.println("Login Failed");
            Serial.println(client.state());
        }
    }
    ui.iot[0] = client.connected();
}

void mqtt_loop()
{
    if (!client.connected())
    {
        mqtt.status = false;
        mqtt_connect();
    }
    else
    {
        mqtt.status = true;
        client.loop();
    }
}

void mqtt_disconnect()
{
    if (mqtt.status == true)
    {
        client.disconnect();
        Serial.println("MQTT Disonnected !");
        mqtt.status = false;
    }
    ui.iot[0] = client.connected();
}