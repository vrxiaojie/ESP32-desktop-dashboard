#include "Wifi_Config.h"

/********************************** WIFI配置 **********************************/

const byte DNS_PORT = 53; // 设置DNS端口号
const int webPort = 80;   // 设置Web端口号

const char *AP_SSID = "ESP32"; // 设置AP热点名称
// const char* AP_PASS  = "";               //这里不设置设置AP热点密码

const char *HOST_NAME = "MY_ESP32"; // 设置设备名
String scanNetworksID = "";         // 用于储存扫描到的WiFi ID

IPAddress apIP(192, 168, 4, 1); // 设置AP的IP地址

String wifi_ssid = ""; // 暂时存储wifi账号密码
String wifi_pass = ""; // 暂时存储wifi账号密码

int connectTimeOut_s = 3; // 超时时间

DNSServer dnsServer;       // 创建dnsServer实例
WebServer server(webPort); // 开启web服务, 创建TCP SERVER,参数: 端口号,最大连接数

WiFiAPClass WiFiAP;

const char *ROOT_HTML = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi 配置</title>
    <style>
    body {
        font-family: Arial, sans-serif;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        background-color: #f4f4f4;
        margin: 0;
    }
    .container {
        background: #D9EDEE;
        padding: 30px;
        border-radius: 16px;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        max-width: 400px;
        width: 100%;
        text-align: center;
    }
    h1 {
        color: #333;
        font-size: 24px;
        margin-bottom: 20px;
    }
    .input {
        margin-top: 20px;
        display: flex;
        align-items: center;
    }
    .input span {
        width: 100px;
        line-height: 36px;
        color: #555;
        text-align: left;
    }
    .input select, .input input {
        height: 36px;
        width: calc(100% - 120px);
        border-radius: 18px;
        border: 1px solid #ccc;
        padding: 0 15px;
        box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.1);
        background-color: #f4f4f4;
        transition: border-color 0.3s ease, box-shadow 0.3s ease;
    }
    .input select:focus, .input input:focus {
        border-color: #3498db;
        box-shadow: 0 0 5px rgba(52, 152, 219, 0.5);
        outline: none;
    }
    .btn {
        width: 100%;
        height: 40px;
        background-color: #3498db;
        border: 0;
        color: white;
        border-radius: 20px;
        margin-top: 30px;
        font-size: 16px;
        cursor: pointer;
        transition: background-color 0.3s ease;
    }
    .btn:hover {
        background-color: #2980b9;
    }
</style>
</head>
<body>
    <div class="container">
        <h1>WiFi 配置</h1>
        <form method="POST" action="configwifi">
            <label class="input">
                <span>WiFi SSID</span>
                <select name="ssid" required>
                    <option value="" disabled selected>选择一个 WiFi 网络</option>
                    <!-- 将 WiFi 网络列表插入到这里 -->
)";

/*
 * 处理网站根目录的访问请求
 */
void handleRoot()
{
  if (server.hasArg("selectSSID"))
  {
    server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>"); // scanNetWprksID是扫描到的wifi
  }
  else
  {
    server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
  }
}

/*
 * 提交数据后的提示页面
 */
void handleConfigWifi() // 返回http状态
{
  if (server.hasArg("ssid")) // 判断是否有账号参数
  {
    Serial.print("got ssid:");
    wifi_ssid = server.arg("ssid"); // 获取html表单输入框name名为"ssid"的内容

    Serial.println(wifi_ssid);
  }
  else // 没有参数
  {
    Serial.println("error, not found ssid");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid"); // 返回错误页面
    return;
  }
  // 密码与账号同理
  if (server.hasArg("pass"))
  {
    Serial.print("got password:");
    wifi_pass = server.arg("pass"); // 获取html表单输入框name名为"pwd"的内容
    Serial.println(wifi_pass);
  }
  else
  {
    Serial.println("error, not found password");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }

  server.send(200, "text/html",
              "<html lang='zh-CN'>"
              "<head>"
              "<meta charset='UTF-8'>"
              "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
              "<title>WiFi 连接中</title>"
              "<style>"
              "body {"
              "font-family: Arial, sans-serif;"
              "display: flex;"
              "justify-content: center;"
              "align-items: center;"
              "height: 100vh;"
              "background-color: #f4f4f4;"
              "margin: 0;"
              "}"
              ".container {"
              "text-align: center;"
              "background: white;"
              "padding: 20px;"
              "border-radius: 8px;"
              "box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);"
              "max-width: 400px;"
              "width: 100%;"
              "}"
              "h1 {"
              "color: #3498db;"
              "font-size: 24px;"
              "margin-bottom: 20px;"
              "}"
              "p {"
              "font-size: 18px;"
              "color: #333;"
              "}"
              "</style>"
              "</head>"
              "<body>"
              "<div class='container'>"
              "<h1>正在连接 WiFi...</h1>"
              "<p>SSID：" +
                  wifi_ssid + "</p>"
                              "<p>已取得WiFi信息，正在尝试连接<br></p>"
                              "<p>若连接成功设备将自动重启。请手动关闭此页面。</p>"
                              "</div>"
                              "</body>"
                              "</html>");

  delay(1000); // 给用户1s时间查看连接信息
  if (connectToWiFi(connectTimeOut_s) == true)
  {
    WiFi.softAPdisconnect(true); // 参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
    server.close();              // 关闭web服务
    Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);
  }
  else
  {
    initSoftAP(); // 若连接失败，再次开启AP模式让用户重连
    initDNS();
    initWebServer();
  }
}

/*
 * 处理404情况的函数'handleNotFound'
 */
void handleNotFound() // 当浏览器请求的网络资源无法在服务器找到时通过此自定义函数处理
{
  handleRoot(); // 访问不存在目录则返回配置页面
}

/*
 * 进入AP模式
 */
void initSoftAP()
{
  WiFi.mode(WIFI_AP);                                           // 配置为AP模式
  WiFiAP.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // 设置AP热点IP和子网掩码
  if (WiFiAP.softAP(AP_SSID))                                   // 开启AP热点,如需要密码则添加第二个参数
  {
    // 打印相关信息
    Serial.println("ESP-32S SoftAP is right.");
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());                                            // 接入点ip
    Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str()); // 接入点mac
  }
  else // 开启AP热点失败
  {
    Serial.println("WiFiAP Failed");
    delay(10);
    Serial.println("restart now...");
    ESP.restart(); // 重启复位esp32
  }
}

/*
 * 开启DNS服务器
 */
void initDNS()
{
  if (dnsServer.start(DNS_PORT, "*", apIP)) // 判断将所有地址映射到esp32的ip上是否成功
  {
    Serial.println("start dnsserver success.");
  }
  else
  {
    Serial.println("start dnsserver failed.");
  }
}

/*
 * 初始化WebServer
 */
void initWebServer()
{
  if (MDNS.begin("esp32")) // 给设备设定域名esp32,完整的域名是esp32.local
  {
    Serial.println("MDNS responder started");
  }
  // 必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);                  //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server.on("/configwifi", HTTP_POST, handleConfigWifi); //  当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理

  server.onNotFound(handleNotFound); // 当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理

  server.begin(); // 启动TCP SERVER

  Serial.println("WebServer started!");
}

/*
 * 扫描附近的WiFi，为了显示在配网界面
 */

bool scanWiFi()
{
  Serial.println("scan start");
  Serial.println("--------->");
  // 扫描附近WiFi
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
    scanNetworksID = "<p>没有找到网络</p>";
    return false;
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    scanNetworksID = ""; // 清空之前的内容
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

      // 将扫描到的每个WiFi名称添加到scanNetworksID中
      //                   <option value="HUAWEI-9FPMBY">HUAWEI-9FPMBY -57dBm</option>
      scanNetworksID += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
    }
    scanNetworksID += (String)("</select>"
                               "</label>"
                               "<label class=\"input\">"
                               "<span>WiFi 密码</span>"
                               "<input type=\"password\" name=\"pass\" required>"
                               " </label>"
                               " <input class=\"btn\" type=\"submit\" name=\"submit\" value=\"提交\">"
                               " </form>"
                               "</div>"
                               "</body>"
                               "</html>");
    return true;
  }
}

/*
 * 连接WiFi
 */
bool connectToWiFi(int timeOut_s)
{
  WiFi.hostname(HOST_NAME); // 设置设备名
  Serial.println("connectToWiFi()");
  WiFi.mode(WIFI_STA);       // 设置为STA模式并连接WIFI
  WiFi.setAutoConnect(true); // 设置自动连接

  if (wifi_ssid != "") // wifi_ssid不为空，意味着从网页读取到wifi
  {
    Serial.println("use web config to connect.");
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str()); // c_str(),获取该字符串的指针
    wifi_ssid = "";
    wifi_pass = "";
  }
  else // 未从网页读取到wifi
  {
    Serial.println("use config stored in NVS to connect.");
    WiFi.begin(); // begin()不传入参数，默认连接上一次连接成功的wifi
  }

  uint8_t Connect_time = 0;             // 用于连接计时，如果长时间连接不成功，复位设备
  while (WiFi.status() != WL_CONNECTED) // 等待WIFI连接成功
  {
    Serial.print(".");
    delay(500);
    Connect_time++;

    if (Connect_time > 2 * timeOut_s) // 长时间连接不上
    {
      Serial.println("WIFI autoconnect fail");
      return false;
    }
  }

  if (WiFi.status() == WL_CONNECTED) // 如果连接成功
  {
    Serial.println("WIFI connect Success");
    Serial.printf("SSID:%s", WiFi.SSID().c_str());
    Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
    Serial.print("LocalIP:");
    Serial.print(WiFi.localIP());
    Serial.print(" ,GateIP:");
    Serial.println(WiFi.gatewayIP());
    Serial.print("WIFI status is:");
    Serial.printf("%d \n", WiFi.status());
    return true;
  }
  return false;
}

/*
 * 删除保存的wifi信息，这里的删除是删除存储在flash的信息。删除后wifi读不到上次连接的记录，需重新配网
 */
void restoreWiFi()
{
  Serial.println("connection config reset");
  delay(20);
  WiFi.eraseAP();
}

/*
 * 检查wifi是否已经连接
 */
void checkConnect(bool reConnect)
{
  if (WiFi.status() != WL_CONNECTED) // wifi连接失败
  {
    if (reConnect == true && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA)
    {
      Serial.println("WIFI未连接.");
      Serial.println("WiFi Mode:");
      Serial.println(WiFi.getMode());
      Serial.println("正在连接WiFi...");
      connectToWiFi(connectTimeOut_s); // 连接wifi函数
    }
  }
}

/*
 * 检测客户端DNS&HTTP请求
 */
void checkDNS_HTTP()
{
  dnsServer.processNextRequest(); // 检查客户端DNS请求
  server.handleClient();          // 检查客户端(浏览器)http请求
}

void reconfig_wifi()
{
  EEPROM_write_single_data("wifi", "reconfig", 0); // 不管最终有没有配置成功，都将标志置零
  reconfig_wifi_show();
  while (1)
  {
    btn_scan();
    if (btn.pressed) // 按下任意键返回到主菜单
    {
      btn.pressed = false;
      return;
    }
    checkDNS_HTTP();                   // 检测客户端DNS&HTTP请求，也就是检查配网页面那部分
    if (WiFi.status() == WL_CONNECTED) // 如果检测到wifi已连接，就退出该页面
      return;
  }
}