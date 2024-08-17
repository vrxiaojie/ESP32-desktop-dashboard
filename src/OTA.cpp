
#include "OTA.h"

WebServer server_OTA(80);

// 上传界面的HTML源码
const char *uploadForm = R"(
    <!DOCTYPE html>
    <html lang="zh-CN">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>ESP32 OTA Update</title>
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
                text-align: center;
                background: white;
                padding: 20px;
                border-radius: 8px;
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                max-width: 400px;
                width: 100%;
            }
            h1 {
                color: #333;
                font-size: 24px;
                margin-bottom: 20px;
            }
            input[type="file"] {
                margin-bottom: 20px;
                padding: 10px;
                border-radius: 5px;
                border: 1px solid #ccc;
                width: 100%;
            }
            input[type="submit"] {
                padding: 10px 20px;
                border-radius: 5px;
                background-color: #3498db;
                color: white;
                border: none;
                cursor: pointer;
                font-size: 16px;
            }
            input[type="submit"]:hover {
                background-color: #2980b9;
            }
            .message {
                margin-top: 20px;
                font-size: 18px;
                color: #333;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>ESP32 OTA Update</h1>
            <p class="description">上传 <span style="color: #e74c3c;">.bin</span> 文件</p>
            <form id="uploadForm" method="POST" action="/update" enctype="multipart/form-data">
                <input type="file" name="update"  accept=".bin"required>
                <input type="submit" value="Update">
            </form>
            <div id="message" class="message hidden"></div>
        </div>
        <script>
            document.getElementById('uploadForm').onsubmit = function() {
                document.getElementById('message').classList.remove('hidden');
                document.getElementById('message').textContent = "固件上传中，请耐心等待";
            }
        </script>
    </body>
    </html>
    )";

void handleOTAUpdate()
{
    HTTPUpload &upload = server_OTA.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin())
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
            Serial.printf("Update Success: %u bytes\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

const char *ssid = "ESP32-OTA";

void OTA_update()
{
    WiFi.softAP(ssid);
    Serial.println("SoftAP started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    server_OTA.on("/", HTTP_GET, []()
                  {
    server_OTA.sendHeader("Connection", "close");
    server_OTA.send(200, "text/html", uploadForm); });

    server_OTA.on("/update", HTTP_POST, []()
                  {
    server_OTA.sendHeader("Connection", "close");
    //显示升级成功or失败的界面
    server_OTA.send(200, "text/html", Update.hasError() ? R"(
        <!DOCTYPE html>
        <html lang="zh-CN">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Update Failed</title>
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
                    text-align: center;
                    background: white;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                    max-width: 400px;
                    width: 100%;
                }
                h1 {
                    color: #e74c3c;
                    font-size: 24px;
                    margin-bottom: 20px;
                }
                p {
                    font-size: 18px;
                    color: #333;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>升级失败！</h1>
                <p>在升级期间遇到了亿点问题,请重新进入OTA界面.</p>
            </div>
        </body>
        </html>
        )" : R"(
        <!DOCTYPE html>
        <html lang="zh-CN">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Update Successful</title>
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
                    text-align: center;
                    background: white;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
                    max-width: 400px;
                    width: 100%;
                }
                h1 {
                    color: #2ecc71;
                    font-size: 24px;
                    margin-bottom: 20px;
                }
                p {
                    font-size: 18px;
                    color: #333;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>升级成功！</h1>
                <p>设备将立即重启</p>
            </div>
        </body>
        </html>
        )");
    delay(1000);
    ESP.restart(); }, handleOTAUpdate);

    server_OTA.begin();

    while (1)
    {
        server_OTA.handleClient();
        btn_scan();
        if (btn.pressed) // 按下任意键返回到主菜单
        {
            btn.pressed = false;
            WiFi.softAPdisconnect(true);
            return;
        }
    }
}
