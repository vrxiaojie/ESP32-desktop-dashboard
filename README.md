<!-- 
徽章放置 使用shields.io
-->
<a href="">
<img alt="Static Badge" src="https://img.shields.io/badge/Repo-ESP32_desktop_dashboard-%23349ad7">
</a>
<a href="">
<img alt="GitHub repo file or directory count" src="https://img.shields.io/github/directory-file-count/vrxiaojie/ESP32-desktop-dashboard">
</a>
<a href="">
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/vrxiaojie/ESP32-desktop-dashboard">
</a>
<a href="">
<img alt="Github Forks" src="https://img.shields.io/github/forks/vrxiaojie/CW32F030C8T6_VAmeter">
</a>
<a href="">
<img alt="GitHub Repo stars" src="https://img.shields.io/github/stars/vrxiaojie/ESP32-desktop-dashboard">
</a>
<a href="">
<img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/vrxiaojie/ESP32-desktop-dashboard">
</a>

<!-- 
正文
-->
# 基于ESP32的桌面仪表盘

## 0 项目开源地址
[https://github.com/vrxiaojie/ESP32-desktop-dashboard](https://github.com/vrxiaojie/ESP32-desktop-dashboard)
[https://oshwhub.com/vrxiaojie/esp32-based-desktop-dashboard](https://oshwhub.com/vrxiaojie/esp32-based-desktop-dashboard)

## 1 简介
使用立创开发板ESP32S3R8N8。

可实时显示电脑各项性能指标，还有温湿度检测、低功耗睡眠、自动亮度、OTA升级等功能。菜单基于WouoUI二次开发，丝滑流畅。

## 2 硬件
硬件部分，需要用到的器件如下
|名称|数量|
|:---:|:---:|
|立创ESP32-S3R8N8开发板|1|
|0.96寸OLED屏 128*64分辨率|1|
|BH1750模块|1|
|SHT40模块(可选)|1|

## 3 软件
### 3.1 电脑端Python程序
```python
import json
import psutil
import GPUtil
import random
import time
import paho.mqtt.client as mqtt_client


def PC_Info():
    msg = dict()
    msg["CPU_usage"] = str(psutil.cpu_percent(interval=1))
    msg["CPU_Frequency"] = str(round(psutil.cpu_freq().current / 1000.0, 1))
    msg["RAM_usage"] = str(psutil.virtual_memory().percent)

    gpu = GPUtil.getGPUs()
    for gpu in gpu:
        msg["GPU_temp"] = str(gpu.temperature)
        msg["GPU_memory"] = str(gpu.memoryUsed)

    msg = json.dumps(msg, ensure_ascii=False)
    # print(msg)
    return msg


# 配置MQTT服务器
host = '192.168.50.111'
port = 1883
# 配置MQTT客户端
topic = "/python/esp32_mqtt"  #创建话题，可以自定义
client_id = f'python-mqtt-{random.randint(0, 100)}'
username = 'python'
password = '123456'


def publish(client, msg):
    result = client.publish(topic, msg)


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.username_pw_set(username, password)
    client.connect(host=host, port=port)
    return client


def run():
    client = connect_mqtt()
    client.loop_start()
    while True:
        msg = str(PC_Info())
        publish(client, msg)
        time.sleep(0.5)


if __name__ == '__main__':
    run()

```

### 3.2 固件烧录
目前有两种方法获取固件：

1. 从GitHub的Release文件获取.bin文件
2. 将GitHub仓库clone下来，手动配置PlatformIO环境，并编译

烧录方法：

1. 用Type-C线将开发板与电脑相连接，使用乐鑫官方提供的烧录工具https://www.espressif.com.cn/zh-hans/support/download/other-tools
2. 用Type-C线将开发板与电脑相连接，在PlatformIO中下载固件到设备
3. (仅成功烧录一遍后使用)在菜单中选择OTA升级，电脑连接设备WIFI，打开网页192.168.4.1，选择在Release下载的.bin文件升级


## 4 3D外壳
使用立创EDA绘制的3D外壳，具体请在开源链接中用编辑器打开查看

建议使用尼龙材质，有点小贵，但外观质感极佳

## 5 外观展示
![IMG_20240822_170629.jpg](https://image.lceda.cn/oshwhub/75b0f974e146487f9f4ea0bb42b3ce13.jpg)

## 6 致谢
UI基于`WouoUI`修改，原作者仓库:https://github.com/RQNG/WouoUI



