
/*
  一、说明：
  此项目核心UI使用的是WouoUI v2，在此基础上做了修改。适配ESP32-S3，编程环境VSCode + PlatformIO
  修改者：VRxiaojie  B站主页：https://space.bilibili.com/11526854
  特别感谢原作者：音游玩的人，B站主页：https://space.bilibili.com/9182439

  二、修改内容：
  - 将原.ino文件拆为.cpp文件+.h文件形式，按功能进行命名、分类，方便用户调用及添加功能
  - 主要适配项目为：基于ESP32的桌面仪表盘，
  开源于立创开源广场：https://oshwhub.com/vrxiaojie/esp32-based-desktop-dashboard
  - 需要用到的外设有：0.96寸 128*64 OLED屏幕；BH1750光照传感器；BME280温湿度气压传感器

  三、一些配置的修改方法：
  1. Wifi_Config.h中可修改SSID和PASSWORD
  2. Mqtt.h 中修改MQTT连接相关参数
*/

/************************************* 包含头文件 *************************************/
#include "BH1750.h"
#include "Timer.h"
#include "Wifi_Config.h"
#include "main.h"
#include "Picture.h"
#include "Pref_EEPROM.h"
#include "Key.h"
#include "Mqtt.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "OTA.h"
#include <driver/rtc_io.h>
/************************************* 屏幕驱动 *************************************/

// 分辨率128*64，可以使用硬件IIC接口

#define SCL 2
#define SDA 3
#define RST U8X8_PIN_NONE

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, RST, SCL, SDA); // 分辨率：128*64  驱动：SSD1306  接口：IIC（硬件）

UI ui; // 实例化UI变量

bool in_setting_list = false;

/************************************* 定义内容 *************************************/

/************************************* 文字内容 *************************************/

M_SELECT main_menu[]{
    {"睡眠"},
    {"IOT管理"},
    {"电压测量"},
    {"监控仪表盘"},
    {"设置"},
    {"OTA升级"},
};

M_SELECT monitor_menu[] // 新增监控菜单
    {
        // 前缀'#'代表是浮点数
        {"[ PC监控仪表盘 ]"},
        {"# CPU使用率 %"},
        {"# 频率(GHz)"},
        {"# RAM %"},
        {"# GPU温度 ℃"},
        {"# VRAM %"},
    };

M_SELECT iot_menu[]{
    {"[ IOT管理 ]"},
    {"+ MQTT"},
    {"+ WIFI"},
    {"- 重新配网"},
};

M_SELECT volt_menu[]{
    {"A0"},
    {"A1"},
};

M_SELECT setting_menu[]{
    // 前缀~代表整数；+代表复选框
    {"[   设置   ]"},
    {"~ 屏幕亮度"},
    {"~ 磁贴动画速度"},
    {"~ 列表动画速度"},
    {"~ 弹窗动画速度"},
    {"~ 聚光动画速度"},
    {"~ 标签动画速度"},
    {"~ 消失动画速度"},
    {"~ 按键短按时长"},
    {"~ 按键长按时长"},
    {"~ CPU主频"},
    {"+ 磁贴从头展开"},
    {"+ 列表从头展开"},
    {"+ 磁贴循环"},
    {"+ 列表循环"},
    {"+ 弹窗背景虚化"},
    {"+ 深色模式"},
    {"+ 自动亮度"},
    {"- [ 关于 ]"},
};

M_SELECT about_menu[]{
    {"[ WouoUI For ESP32 ]"},
    {"- 版本: v2.3"},
    {"- 开发板: ESP32-S3"},
    {"~ CPU主频 MHz:"},
    {"- RAM: 320KB"},
    {"- Flash: 8MB"},
    {"- UI原作者: RQNG"},
    {"- BiliBili: 9182439"},
    {"- 修改者: VRxiaojie"},
    {"- BiliBili: 11526854"}};

/********************************** 页面变量 **********************************/
// OLED变量
uint8_t *buf_ptr; // 指向屏幕缓冲的指针
uint16_t buf_len; // 缓冲长度

// 电压测量页面变量
// 开发板模拟引脚
uint8_t analog_pin[10] = {15, 16};

/************************************ 初始化函数 ***********************************/

/********************************* 初始化数据处理函数 *******************************/
// 显示浮点数的初始化
void check_box_d_init(double *param)
{
  check_box.d = param;
}

// 显示数值的初始化
void check_box_v_init(uint8_t *param)
{
  check_box.v = param;
}

// 多选框的初始化
void check_box_m_init(uint8_t *param)
{
  check_box.m = param;
}

// 单选框时的初始化
void check_box_s_init(uint8_t *param, uint8_t *param_p)
{
  check_box.s = param;
  check_box.s_p = param_p;
}

// 多选框处理函数
void check_box_m_select(uint8_t param)
{
  check_box.m[param] = !check_box.m[param];
  eeprom.change = in_setting_list ? true : false;
}

// 单选框处理函数
void check_box_s_select(uint8_t val, uint8_t pos)
{
  *check_box.s = val;
  *check_box.s_p = pos;
  eeprom.change = in_setting_list ? true : false;
}

// 弹窗数值初始化
void window_value_init(const char title[], uint8_t select, uint8_t *value, uint8_t max, uint8_t min, uint8_t step, MENU *bg, uint8_t index)
{
  strcpy(win.title, title);
  win.select = select;
  win.value = value;
  win.max = max;
  win.min = min;
  win.step = step;
  win.bg = bg;
  win.index = index;
  ui.index = M_WINDOW;
  ui.state = S_WINDOW;
}

/*********************************** UI 初始化函数 *********************************/

// 在初始化EEPROM时，选择性初始化的默认设置
void ui_param_init()
{
  ui.param[DISP_BRI] = 255; // 屏幕亮度
  ui.param[TILE_ANI] = 30;  // 磁贴动画速度
  ui.param[LIST_ANI] = 60;  // 列表动画速度
  ui.param[WIN_ANI] = 25;   // 弹窗动画速度
  ui.param[SPOT_ANI] = 50;  // 聚光动画速度
  ui.param[TAG_ANI] = 60;   // 标签动画速度
  ui.param[FADE_ANI] = 30;  // 消失动画速度
  ui.param[BTN_SPT] = 25;   // 按键短按时长
  ui.param[BTN_LPT] = 150;  // 按键长按时长
  ui.param[CPU_FREQ] = 240; // ESP32 CPU主频
  ui.param[TILE_UFD] = 1;   // 磁贴图标从头展开开关
  ui.param[LIST_UFD] = 1;   // 菜单列表从头展开开关
  ui.param[TILE_LOOP] = 0;  // 磁贴图标循环模式开关
  ui.param[LIST_LOOP] = 0;  // 菜单列表循环模式开关
  ui.param[WIN_BOK] = 0;    // 弹窗背景虚化开关
  ui.param[DARK_MODE] = 1;  // 黑暗模式开关
  ui.param[AUTO_BRI] = 1;   // 自动亮度开关
}

// 列表类页面列表行数初始化，必须初始化的参数
void ui_init()
{
  ui.num[M_MAIN] = sizeof(main_menu) / sizeof(M_SELECT);
  ui.num[M_IOT] = sizeof(iot_menu) / sizeof(M_SELECT);
  ui.num[M_VOLT] = sizeof(volt_menu) / sizeof(M_SELECT);
  ui.num[M_SETTING] = sizeof(setting_menu) / sizeof(M_SELECT);
  ui.num[M_MONITOR] = sizeof(monitor_menu) / sizeof(M_SELECT);
  ui.num[M_ABOUT] = sizeof(about_menu) / sizeof(M_SELECT);
}

/********************************* 分页面初始化函数 ********************************/

// 进入磁贴类时的初始化
void tile_param_init()
{
  ui.init = false;
  tile.icon_x = 0;
  tile.icon_x_trg = TILE_ICON_S;
  tile.icon_y = -TILE_ICON_H;
  tile.icon_y_trg = 0;
  tile.indi_x = 0;
  tile.indi_x_trg = TILE_INDI_W;
  tile.title_y = tile.title_y_calc;
  tile.title_y_trg = tile.title_y_trg_calc;
}

// 进入睡眠时的初始化
void sleep_param_init()
{
  u8g2.setPowerSave(1); // 将屏幕关闭
  ui.state = S_NONE;
  ui.sleep = true;
  if (eeprom.change)
  {
    EEPROM_write_data(); // 将设置参数写入EEPROM
    eeprom.change = false;
  }
  // TODO: GPIO14 待修改为实际板子上的确认键
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 1); // GPIO14，即确认键，待板子到了之后再修改
  rtc_gpio_pullup_dis(GPIO_NUM_14);
  rtc_gpio_pulldown_en(GPIO_NUM_14);
  esp_deep_sleep_start();       // 进入深度睡眠
  rtc_gpio_deinit(GPIO_NUM_14); // 从睡眠中唤醒，并把GPIO14 RTC去初始化
  pinMode(14, INPUT_PULLDOWN);  // 将GPIO14 重新初始化为数字GPIO
}

// 电压测量页初始化
void volt_param_init()
{
  volt.text_bg_r = 0;
  volt.text_bg_r_trg = VOLT_TEXT_BG_W;
}

// 设置页初始化
void setting_param_init()
{
  check_box_v_init(ui.param);
  check_box_m_init(ui.param);
  in_setting_list = true;
}

// IOT管理页初始化
void iot_param_init()
{
  ui.iot[0] = mqtt.status;
  ui.iot[1] = (WiFi.status() == WL_CONNECTED) ? 1 : 0;
  check_box_m_init(ui.iot);
}

// 监控菜单初始化
void monitor_param_init()
{
  check_box_d_init(ui.PC_monitor);
}

// 关于页初始化
void about_param_init()
{
  check_box_v_init(ui.about);
}

/********************************** 通用初始化函数 *********************************/

/*
  页面层级管理逻辑是，把所有页面都先当作列表类初始化，不是列表类按需求再初始化对应函数
  这样做会浪费一些资源，但跳转页面时只需要考虑页面层级，逻辑上更清晰，减少出错
*/

// 弹窗动画初始化
void window_param_init()
{
  win.bar = 0;
  win.y = WIN_Y;
  win.y_trg = win.u;
  ui.state = S_NONE;
}

// 进入更深层级时的初始化
void layer_init_in()
{
  ui.layer++;
  ui.init = 0;
  list.y = 0;
  list.y_trg = LIST_LINE_H;
  list.box_x = 0;
  list.box_y = 0;
  list.bar_y = 0;
  ui.state = S_FADE;
  switch (ui.index)
  {
  case M_MAIN:
    tile_param_init();
    break; // 睡眠进入主菜单，动画初始化
  case M_VOLT:
    volt_param_init();
    break; // 主菜单进入电压测量页，动画初始化
  case M_MONITOR:
    monitor_param_init();
    break;    // 新增主菜单进入监控页，动画初始化
  case M_IOT: // 主菜单进入IOT管理页，动画初始化
    iot_param_init();
    break;
  case M_SETTING:
    setting_param_init();
    break; // 主菜单进入设置页，单选框初始化
  case M_ABOUT:
    about_param_init();
    break; // 进入关于页，单选框、数值初始化
  }
}

// 进入更浅层级时的初始化
void layer_init_out()
{
  ui.select[ui.layer] = 0;
  list.box_y_trg[ui.layer] = 0;
  ui.layer--;
  ui.init = 0;
  list.y = 0;
  list.y_trg = LIST_LINE_H;
  list.bar_y = 0;
  ui.state = S_FADE;
  switch (ui.index)
  {
  case M_SETTING:
    setting_param_init();
    break; // 从关于页进入设置页，单选框初始化
  case M_SLEEP:
    sleep_param_init();
    break; // 主菜单进入睡眠页，检查是否需要写EEPROM
  case M_MAIN:
    tile_param_init();
    if (eeprom.change)
    {
      EEPROM_write_data();
      eeprom.change = false;
    }
    break; // 不管什么页面进入主菜单时，动画初始化
  }
}

/************************************* 动画函数 *************************************/

// 动画函数
void animation(float *a, float *a_trg, uint8_t n)
{
  if (*a != *a_trg)
  {
    if (fabs(*a - *a_trg) < 0.15f)
      *a = *a_trg;
    else
      *a += (*a_trg - *a) / (ui.param[n] / 10.0f);
  }
}

// 消失函数
void fade()
{
  delay(ui.param[FADE_ANI]);
  if (ui.param[DARK_MODE])
  {
    switch (ui.fade)
    {
    case 1:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 != 0)
          buf_ptr[i] = buf_ptr[i] & 0xAA;
      break;
    case 2:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 != 0)
          buf_ptr[i] = buf_ptr[i] & 0x00;
      break;
    case 3:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 == 0)
          buf_ptr[i] = buf_ptr[i] & 0x55;
      break;
    case 4:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 == 0)
          buf_ptr[i] = buf_ptr[i] & 0x00;
      break;
    default:
      ui.state = S_NONE;
      ui.fade = 0;
      break;
    }
  }
  else
  {
    switch (ui.fade)
    {
    case 1:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 != 0)
          buf_ptr[i] = buf_ptr[i] | 0xAA;
      break;
    case 2:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 != 0)
          buf_ptr[i] = buf_ptr[i] | 0x00;
      break;
    case 3:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 == 0)
          buf_ptr[i] = buf_ptr[i] | 0x55;
      break;
    case 4:
      for (uint16_t i = 0; i < buf_len; ++i)
        if (i % 2 == 0)
          buf_ptr[i] = buf_ptr[i] | 0x00;
      break;
    default:
      ui.state = S_NONE;
      ui.fade = 0;
      break;
    }
  }
  ui.fade++;
}

/************************************* 显示函数 *************************************/

// 磁贴类页面通用显示函数
void tile_show(struct MENU arr_1[], const uint8_t icon_pic[][120])
{
  // 计算动画过渡值
  animation(&tile.icon_x, &tile.icon_x_trg, TILE_ANI);
  animation(&tile.icon_y, &tile.icon_y_trg, TILE_ANI);
  animation(&tile.indi_x, &tile.indi_x_trg, TILE_ANI);
  animation(&tile.title_y, &tile.title_y_trg, TILE_ANI);

  // 设置大标题的颜色，0透显，1实显，2反色，这里用实显
  u8g2.setDrawColor(1);
  // 绘制大标题 中文 字号16
  u8g2.setFont(TILE_B_FONT);
  // Serial.print(u8g2.getUTF8Width(arr_1[ui.select[ui.layer]].m_select));
  u8g2.drawUTF8((DISP_W - u8g2.getUTF8Width(arr_1[ui.select[ui.layer]].m_select)) / 2, tile.title_y, arr_1[ui.select[ui.layer]].m_select);
  // 绘制大标题指示器
  u8g2.drawBox(0, TILE_ICON_S, tile.indi_x, TILE_INDI_H);

  // 绘制图标
  if (!ui.init)
  {
    for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
    {
      if (ui.param[TILE_UFD])
        tile.temp = (DISP_W - TILE_ICON_W) / 2 + i * tile.icon_x - TILE_ICON_S * ui.select[ui.layer];
      else
        tile.temp = (DISP_W - TILE_ICON_W) / 2 + (i - ui.select[ui.layer]) * tile.icon_x;
      u8g2.drawXBMP(tile.temp, (int16_t)tile.icon_y, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);
    }
    if (tile.icon_x == tile.icon_x_trg)
    {
      ui.init = true;
      tile.icon_x = tile.icon_x_trg = -ui.select[ui.layer] * TILE_ICON_S;
    }
  }
  else
    for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
      u8g2.drawXBMP((DISP_W - TILE_ICON_W) / 2 + (int16_t)tile.icon_x + i * TILE_ICON_S, 0, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);

  // 反转屏幕内元素颜色，白天模式遮罩
  u8g2.setDrawColor(2);
  if (!ui.param[DARK_MODE])
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

/*************** 根据列表每行开头符号，判断每行尾部是否绘制以及绘制什么内容 *************/
// 列表显示浮点数值
void list_draw_double_value(int n) { u8g2.print(check_box.d[n - 1]); }

// 列表显示数值
void list_draw_value(int n) { u8g2.print(check_box.v[n - 1]); }

// 绘制外框
void list_draw_check_box_frame() { u8g2.drawRFrame(CHECK_BOX_L_S, list.temp + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, 1); }

// 绘制框里面的点
void list_draw_check_box_dot() { u8g2.drawBox(CHECK_BOX_L_S + CHECK_BOX_D_S + 1, list.temp + CHECK_BOX_U_S + CHECK_BOX_D_S + 1, CHECK_BOX_F_W - (CHECK_BOX_D_S + 1) * 2, CHECK_BOX_F_H - (CHECK_BOX_D_S + 1) * 2); }

// 判断列表尾部内容
void list_draw_text_and_check_box(struct MENU arr[], int i)
{
  u8g2.drawUTF8(LIST_TEXT_S, list.temp + LIST_TEXT_H + LIST_TEXT_S, arr[i].m_select);
  u8g2.setCursor(CHECK_BOX_L_S, list.temp + LIST_TEXT_H + LIST_TEXT_S);
  switch (arr[i].m_select[0])
  {
  case '~':
    list_draw_value(i);
    break;
  case '+':
    list_draw_check_box_frame();
    if (check_box.m[i - 1] == 1)
      list_draw_check_box_dot();
    break;
  case '=':
    list_draw_check_box_frame();
    if (*check_box.s_p == i)
      list_draw_check_box_dot();
    break;
  case '#':
    list_draw_double_value(i);
    break;
  }
}

/******************************** 列表显示函数 **************************************/

// 列表类页面通用显示函数
void list_show(struct MENU arr[], uint8_t ui_index)
{
  // 更新动画目标值
  u8g2.setFont(LIST_FONT);
  list.box_x_trg = u8g2.getUTF8Width(arr[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;
  list.bar_y_trg = ceil((ui.select[ui.layer]) * ((float)DISP_H / (ui.num[ui_index] - 1)));

  // 计算动画过渡值
  animation(&list.y, &list.y_trg, LIST_ANI);
  animation(&list.box_x, &list.box_x_trg, LIST_ANI);
  animation(&list.box_y, &list.box_y_trg[ui.layer], LIST_ANI);
  animation(&list.bar_y, &list.bar_y_trg, LIST_ANI);

  // 检查循环动画是否结束
  if (list.loop && list.box_y == list.box_y_trg[ui.layer])
    list.loop = false;

  // 设置文字和进度条颜色，0透显，1实显，2反色，这里都用实显
  u8g2.setDrawColor(1);

  // 绘制进度条
  u8g2.drawHLine(DISP_W - LIST_BAR_W, 0, LIST_BAR_W);
  u8g2.drawHLine(DISP_W - LIST_BAR_W, DISP_H - 1, LIST_BAR_W);
  u8g2.drawVLine(DISP_W - ceil((float)LIST_BAR_W / 2), 0, DISP_H);
  u8g2.drawBox(DISP_W - LIST_BAR_W, 0, LIST_BAR_W, list.bar_y);

  // 绘制列表文字
  if (!ui.init)
  {
    for (int i = 0; i < ui.num[ui_index]; ++i)
    {
      if (ui.param[LIST_UFD])
        list.temp = i * list.y - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
      else
        list.temp = (i - ui.select[ui.layer]) * list.y + list.box_y_trg[ui.layer];
      list_draw_text_and_check_box(arr, i);
    }
    if (list.y == list.y_trg)
    {
      ui.init = true;
      list.y = list.y_trg = -LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
    }
  }
  else
    for (int i = 0; i < ui.num[ui_index]; ++i)
    {
      list.temp = LIST_LINE_H * i + list.y;
      list_draw_text_and_check_box(arr, i);
    }

  // 绘制文字选择框，0透显，1实显，2反色，这里用反色
  u8g2.setDrawColor(2);
  u8g2.drawRBox(0, list.box_y, list.box_x, LIST_LINE_H, LIST_BOX_R);

  // 反转屏幕内元素颜色，白天模式遮罩，在这里屏蔽有列表参与的页面，使遮罩作用在那个页面上
  if (!ui.param[DARK_MODE])
  {
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
    switch (ui.index)
    {
    case M_WINDOW:
    case M_VOLT:
      u8g2.drawBox(0, 0, DISP_W, DISP_H);
    }
  }
}

// 电压页面显示函数
void volt_show()
{
  // 使用列表类显示选项
  list_show(volt_menu, M_VOLT);

  // 计算动画过渡值
  animation(&volt.text_bg_r, &volt.text_bg_r_trg, TAG_ANI);

  // 设置曲线颜色，0透显，1实显，2反色，这里用实显
  u8g2.setDrawColor(1);

  // 绘制电压曲线和外框
  volt.val = 0;
  u8g2.drawFrame(WAVE_BOX_L_S, 0, WAVE_BOX_W, WAVE_BOX_H);
  u8g2.drawFrame(WAVE_BOX_L_S + 1, 1, WAVE_BOX_W - 2, WAVE_BOX_H - 2);
  if (list.box_y == list.box_y_trg[ui.layer] && list.y == list.y_trg)
  {
    for (int i = 0; i < WAVE_SAMPLE * WAVE_W; i++)
      volt.ch0_adc[i] = volt.val = analogRead(analog_pin[ui.select[ui.layer]]);
    for (int i = 1; i < WAVE_W - 1; i++)
    {
      volt.ch0_wave[i] = map(volt.ch0_adc[int(5 * i)], 0, 4095, WAVE_MAX, WAVE_MIN);
      u8g2.drawLine(WAVE_L + i - 1, WAVE_U + volt.ch0_wave[i - 1], WAVE_L + i, WAVE_U + volt.ch0_wave[i]);
    }
  }

  // 绘制值
  u8g2.setFontDirection(0);
  u8g2.setFont(VOLT_FONT);
  u8g2.setCursor(39, DISP_H - 6);
  u8g2.print(volt.val / 4096.0f * 3.3f);
  u8g2.print("V");

  // 绘制列表选择框和电压文字背景
  u8g2.setDrawColor(2);
  u8g2.drawBox(VOLT_TEXT_BG_L_S, DISP_H - VOLT_TEXT_BG_H, volt.text_bg_r, VOLT_TEXT_BG_H);

  // 反转屏幕内元素颜色，白天模式遮罩
  if (!ui.param[DARK_MODE])
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

// 弹窗通用显示函数
void window_show()
{
  // 绘制背景列表，根据开关判断背景是否要虚化
  list_show(win.bg, win.index);
  if (ui.param[WIN_BOK])
    for (uint16_t i = 0; i < buf_len; ++i)
      buf_ptr[i] = buf_ptr[i] & (i % 2 == 0 ? 0x55 : 0xAA);

  // 更新动画目标值
  u8g2.setFont(WIN_FONT);
  win.bar_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (WIN_BAR_W - 4);

  // 计算动画过渡值
  animation(&win.bar, &win.bar_trg, WIN_ANI);
  animation(&win.y, &win.y_trg, WIN_ANI);

  // 绘制窗口
  u8g2.setDrawColor(0);
  u8g2.drawRBox(win.l, (int16_t)win.y, WIN_W, WIN_H, 2); // 绘制外框背景
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);                  // 绘制外框描边
  u8g2.drawRFrame(win.l + 5, (int16_t)win.y + 20, WIN_BAR_W, WIN_BAR_H, 1); // 绘制进度条外框
  u8g2.drawBox(win.l + 7, (int16_t)win.y + 22, win.bar, WIN_BAR_H - 4);     // 绘制进度条
  u8g2.drawUTF8(win.l + 5, (int16_t)win.y + 14, win.title);                 // 绘制标题
  u8g2.setCursor(win.l + 78, (int16_t)win.y + 14);
  u8g2.print(*win.value); // 绘制当前值

  // 需要在窗口修改参数时立即见效的函数 亮度、CPU主频
  if (strstr(win.title, "屏幕亮度") != NULL)
    u8g2.setContrast(ui.param[DISP_BRI]);
  if (strstr(win.title, "CPU主频") != NULL)
    setCpuFrequencyMhz(ui.param[CPU_FREQ]);

  // 反转屏幕内元素颜色，白天模式遮罩
  u8g2.setDrawColor(2);
  if (!ui.param[DARK_MODE])
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

// WIFI配网显示
void reconfig_wifi_show()
{
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  u8g2.drawUTF8(48, 16, "[配网]");

  u8g2.drawUTF8(4, 32, "扫描WIFI...");
  u8g2.sendBuffer();
  initSoftAP();
  initDNS();
  initWebServer();
  scanWiFi();
  u8g2.clearBuffer();
  u8g2.drawUTF8(4, 14, "[配网]按任意键可退出");
  u8g2.drawUTF8(4, 30, "手机连到热点:ESP32");
  u8g2.drawUTF8(4, 46, "进入网页:192.168.4.1");
  u8g2.drawUTF8(4, 62, "成功后该页自动关闭");
  u8g2.sendBuffer();
}

/************************************* 处理函数 *************************************/

/*********************************** 通用处理函数 ***********************************/

// 磁贴类页面旋转时判断通用函数
void tile_rotate_switch()
{
  switch (btn.id)
  {
  case BTN_ID_CC:
    if (ui.init)
    {
      if (ui.select[ui.layer] > 0)
      {
        ui.select[ui.layer] -= 1;
        tile.icon_x_trg += TILE_ICON_S;
        tile.select_flag = false;
      }
      else
      {
        if (ui.param[TILE_LOOP])
        {
          ui.select[ui.layer] = ui.num[ui.index] - 1;
          tile.icon_x_trg = -TILE_ICON_S * (ui.num[ui.index] - 1);
          break;
        }
        else
          tile.select_flag = true;
      }
    }
    break;

  case BTN_ID_CW:
    if (ui.init)
    {
      if (ui.select[ui.layer] < (ui.num[ui.index] - 1))
      {
        ui.select[ui.layer] += 1;
        tile.icon_x_trg -= TILE_ICON_S;
        tile.select_flag = false;
      }
      else
      {
        if (ui.param[TILE_LOOP])
        {
          ui.select[ui.layer] = 0;
          tile.icon_x_trg = 0;
          break;
        }
        else
          tile.select_flag = true;
      }
    }
    break;
  }
}

// 列表类页面旋转时判断通用函数
void list_rotate_switch()
{
  if (!list.loop)
  {
    switch (btn.id)
    {
    case BTN_ID_CC:
      if (ui.select[ui.layer] == 0)
      {
        if (ui.param[LIST_LOOP] && ui.init)
        {
          list.loop = true;
          ui.select[ui.layer] = ui.num[ui.index] - 1;
          if (ui.num[ui.index] > list.line_n)
          {
            list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
            list.y_trg = DISP_H - ui.num[ui.index] * LIST_LINE_H;
          }
          else
            list.box_y_trg[ui.layer] = (ui.num[ui.index] - 1) * LIST_LINE_H;
          break;
        }
        else
          break;
      }
      if (ui.init)
      {
        ui.select[ui.layer] -= 1;
        if (ui.select[ui.layer] < -(list.y_trg / LIST_LINE_H))
        {
          if (!(DISP_H % LIST_LINE_H))
            list.y_trg += LIST_LINE_H;
          else
          {
            if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * list.line_n)
            {
              list.y_trg += (list.line_n + 1) * LIST_LINE_H - DISP_H;
              list.box_y_trg[ui.layer] = 0;
            }
            else if (list.box_y_trg[ui.layer] == LIST_LINE_H)
            {
              list.box_y_trg[ui.layer] = 0;
            }
            else
              list.y_trg += LIST_LINE_H;
          }
        }
        else
          list.box_y_trg[ui.layer] -= LIST_LINE_H;
        break;
      }

    case BTN_ID_CW:
      if (ui.select[ui.layer] == (ui.num[ui.index] - 1))
      {
        if (ui.param[LIST_LOOP] && ui.init)
        {
          list.loop = true;
          ui.select[ui.layer] = 0;
          list.y_trg = 0;
          list.box_y_trg[ui.layer] = 0;
          break;
        }
        else
          break;
      }
      if (ui.init)
      {
        ui.select[ui.layer] += 1;
        if ((ui.select[ui.layer] + 1) > (list.line_n - list.y_trg / LIST_LINE_H))
        {
          if (!(DISP_H % LIST_LINE_H))
            list.y_trg -= LIST_LINE_H;
          else
          {
            if (list.box_y_trg[ui.layer] == LIST_LINE_H * (list.line_n - 1))
            {
              list.y_trg -= (list.line_n + 1) * LIST_LINE_H - DISP_H;
              list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
            }
            else if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * 2)
            {
              list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
            }
            else
              list.y_trg -= LIST_LINE_H;
          }
        }
        else
          list.box_y_trg[ui.layer] += LIST_LINE_H;
        break;
      }
      break;
    }
  }
}

// 弹窗通用处理函数
void window_proc()
{
  window_show();
  if (win.y == WIN_Y_TRG)
    ui.index = win.index;
  if (btn.pressed && win.y == win.y_trg && win.y != WIN_Y_TRG)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
      if (*win.value < win.max)
        *win.value += win.step;
      eeprom.change = in_setting_list ? true : false;
      break;
    case BTN_ID_CC:
      if (*win.value > win.min)
        *win.value -= win.step;
      eeprom.change = in_setting_list ? true : false;
      break;
    case BTN_ID_SP:
    case BTN_ID_LP:
      win.y_trg = WIN_Y_TRG;
      break;
    }
  }
}

/********************************** 分页面处理函数 **********************************/

// 睡眠页面处理函数
void sleep_proc()
{
  u8g2.setPowerSave(0); // 开启屏幕
  ui.index = M_MAIN;
  ui.state = S_LAYER_IN;
  ui.sleep = false;
}

// 主菜单处理函数，磁贴类模板
void main_proc()
{
  tile_show(main_menu, main_icon_pic);
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      tile_rotate_switch();
      break;
    case BTN_ID_SP:
      switch (ui.select[ui.layer])
      {

      case 0:
        ui.index = M_SLEEP;
        ui.state = S_LAYER_OUT;
        break;
      case 1:
        ui.index = M_IOT;
        ui.state = S_LAYER_IN;
        break;
      case 2:
        ui.index = M_VOLT;
        ui.state = S_LAYER_IN;
        break;
      case 3:
        ui.index = M_MONITOR;
        ui.state = S_LAYER_IN;
        break; // 新增 监控
      case 4:
        ui.index = M_SETTING;
        ui.state = S_LAYER_IN;
        break;
      case 5:
        ui.index = M_OTA;
        ui.state = S_LAYER_IN;
        break;
      }
    }
    if (!tile.select_flag && ui.init)
    {
      tile.indi_x = 0;
      tile.title_y = tile.title_y_calc;
    }
  }
}

// IOT管理菜单处理函数
void iot_proc()
{
  list_show(iot_menu, M_IOT);
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      list_rotate_switch();
      break;
    case BTN_ID_LP:
      ui.select[ui.layer] = 0;
    case BTN_ID_SP:
      switch (ui.select[ui.layer])
      {
      case 0:
        ui.index = M_MAIN;
        ui.state = S_LAYER_OUT;
        break;
      case 1:
        check_box_m_select(0);
        if (mqtt.status == 1)
        {
          mqtt.mqtt_maunual_flag = 1;
          mqtt_disconnect();
        }
        else
        {
          mqtt.mqtt_maunual_flag = 0;
          mqtt.retry_cnt = 0;
          mqtt_connect();
        }
        mqtt.status = ui.iot[0];
        break;
      case 2:
        check_box_m_select(1);
        if (WiFi.status() == WL_CONNECTED)
        {
          if (WiFi.disconnect(true, false)) // WiFi.disconnect(关闭wifi射频 true,擦除WIFI配置 false)
            ui.iot[1] = 0;
        }
        else
        {
          connectToWiFi(connectTimeOut_s);
          ui.iot[1] = (WiFi.status() == WL_CONNECTED) ? 1 : 0;
        }
        break;
      case 3:
        restoreWiFi();                                   // 重新配网。点击后须重启ESP32 然后进到配网界面
        EEPROM_write_single_data("wifi", "reconfig", 1); // 将重新配网值设为1并存入EEPROM
        ESP.restart();
        break;
      }
    }
  }
}

// 新增 监控处理函数
void monitor_proc()
{
  list_show(monitor_menu, M_MONITOR);
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      list_rotate_switch();
      break;
    case BTN_ID_LP:
      ui.select[ui.layer] = 0;
    case BTN_ID_SP:
      switch (ui.select[ui.layer])
      {
      case 0:
        ui.index = M_MAIN;
        ui.state = S_LAYER_OUT;
        break;
      }
    }
  }
}

// 电压测量页处理函数
void volt_proc()
{
  volt_show();
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      list_rotate_switch();
      break;

    case BTN_ID_SP:
    case BTN_ID_LP:
      ui.index = M_MAIN;
      ui.state = S_LAYER_OUT;
      break;
    }
  }
}

// 设置菜单处理函数，多选框列表类模板，弹窗模板
void setting_proc()
{
  list_show(setting_menu, M_SETTING);
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      list_rotate_switch();
      break;
    case BTN_ID_LP:
      ui.select[ui.layer] = 0;
    case BTN_ID_SP:
      switch (ui.select[ui.layer])
      {

      // 返回更浅层级，长按被当作选择这一项，也是执行这一行
      case 0:
        ui.index = M_MAIN;
        ui.state = S_LAYER_OUT;
        in_setting_list = false;
        break;

      // 弹出窗口，参数初始化：标题，参数名，参数值，最大值，最小值，步长，背景列表名，背景列表标签
      case 1:
        window_value_init("屏幕亮度", DISP_BRI, &ui.param[DISP_BRI], 255, 0, 5, setting_menu, M_SETTING);
        break;
      case 2:
        window_value_init("磁贴动画速度", TILE_ANI, &ui.param[TILE_ANI], 100, 10, 1, setting_menu, M_SETTING);
        break;
      case 3:
        window_value_init("列表动画速度", LIST_ANI, &ui.param[LIST_ANI], 100, 10, 1, setting_menu, M_SETTING);
        break;
      case 4:
        window_value_init("弹窗动画速度", WIN_ANI, &ui.param[WIN_ANI], 100, 10, 1, setting_menu, M_SETTING);
        break;
      case 5:
        window_value_init("聚光动画速度", SPOT_ANI, &ui.param[SPOT_ANI], 100, 10, 1, setting_menu, M_SETTING);
        break;
      case 6:
        window_value_init("标签动画速度", TAG_ANI, &ui.param[TAG_ANI], 100, 10, 1, setting_menu, M_SETTING);
        break;
      case 7:
        window_value_init("消失动画速度", FADE_ANI, &ui.param[FADE_ANI], 255, 0, 1, setting_menu, M_SETTING);
        break;
      case 8:
        window_value_init("按键短按时长", BTN_SPT, &ui.param[BTN_SPT], 255, 0, 1, setting_menu, M_SETTING);
        break;
      case 9:
        window_value_init("按键长按时长", BTN_LPT, &ui.param[BTN_LPT], 255, 0, 1, setting_menu, M_SETTING);
        break;
      case 10:
        window_value_init("CPU主频", CPU_FREQ, &ui.param[CPU_FREQ], 240, 80, 80, setting_menu, M_SETTING);
        break;

      // 多选框
      case 11:
        check_box_m_select(TILE_UFD);
        break;
      case 12:
        check_box_m_select(LIST_UFD);
        break;
      case 13:
        check_box_m_select(TILE_LOOP);
        break;
      case 14:
        check_box_m_select(LIST_LOOP);
        break;
      case 15:
        check_box_m_select(WIN_BOK);
        break;
      case 16:
        check_box_m_select(DARK_MODE);
        break;
      case 17:
        check_box_m_select(AUTO_BRI);
        break;

      // 关于本机
      case 18:
        ui.index = M_ABOUT;
        ui.state = S_LAYER_IN;
        break;
      }
    }
  }
}

// 关于本机页
void about_proc()
{
  ui.about[2] = getCpuFrequencyMhz();
  list_show(about_menu, M_ABOUT);
  if (btn.pressed)
  {
    btn.pressed = false;
    switch (btn.id)
    {
    case BTN_ID_CW:
    case BTN_ID_CC:
      list_rotate_switch();
      break;
    case BTN_ID_LP:
      ui.select[ui.layer] = 0;
    case BTN_ID_SP:
      switch (ui.select[ui.layer])
      {

      case 0:
        ui.index = M_SETTING;
        ui.state = S_LAYER_OUT;
        break;
      }
    }
  }
}

#include <qrcode.h>
void ota_proc()
{
  // TODO: 添加二维码用于扫码连接WIFI
  // 生成手机可扫码连接WIFI的二维码
  QRCode qrcode; // 实例化QRCode类
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, "WIFI:T:nopass;S:ESP32-OTA;P:;;"); // 初始化二维码显示的字符
  // get the draw starting point,128 and 64 is screen size
  uint8_t x0 = 3;
  uint8_t y0 = (DISP_H - qrcode.size * 2) / 2;
  // u8g2.setDrawColor(1); //
  u8g2.drawBox(0, 0, 128, 64); // 画箱
  // get QR code pixels in a loop
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      // Check this point is black or white
      if (qrcode_getModule(&qrcode, x, y))
      {
        u8g2.setColorIndex(0);
      }
      else
      {
        u8g2.setColorIndex(1);
      }
      // Double the QR code pixels
      u8g2.drawPixel(x0 + x * 2, y0 + y * 2);
      u8g2.drawPixel(x0 + 1 + x * 2, y0 + y * 2);
      u8g2.drawPixel(x0 + x * 2, y0 + 1 + y * 2);
      u8g2.drawPixel(x0 + 1 + x * 2, y0 + 1 + y * 2);
    }
  }

  // u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  // u8g2.drawUTF8((DISP_W - u8g2.getUTF8Width("OTA升级")) / 2, 44, "OTA升级");
  // u8g2.drawUTF8((DISP_W - u8g2.getUTF8Width(SYS_VERSION)) / 2, 62, SYS_VERSION);
  // u8g2.drawXBMP(49, 0, 30, 30, main_icon_pic[5]);
  // 反转屏幕内元素颜色，白天模式遮罩
  u8g2.setDrawColor(2);
  if (!ui.param[DARK_MODE])
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
  u8g2.sendBuffer();
  OTA_update(); // 启动OTA升级函数，打开AP模式、启动网页服务器
  // 退出上面函数后返回主菜单
  ui.index = M_MAIN;
  ui.state = S_LAYER_OUT;
}

// 总的UI进程
void ui_proc()
{
  u8g2.sendBuffer();
  switch (ui.state)
  {
  case S_FADE:
    fade();
    break; // 转场动画
  case S_WINDOW:
    window_param_init();
    break; // 弹窗初始化
  case S_LAYER_IN:
    layer_init_in();
    break; // 层级初始化
  case S_LAYER_OUT:
    layer_init_out();
    break; // 层级初始化

  case S_NONE:
    u8g2.clearBuffer();
    switch (ui.index) // 直接选择页面
    {
    case M_WINDOW:
      window_proc();
      break;
    case M_SLEEP:
      sleep_proc();
      break;
    case M_MAIN:
      main_proc();
      break;
    case M_IOT:
      iot_proc();
      break;
    case M_VOLT:
      volt_proc();
      break;
    case M_MONITOR:
      monitor_proc();
      break; // 新增 monitor处理函数调用
    case M_SETTING:
      setting_proc();
      break;
    case M_ABOUT:
      about_proc();
      break;
    case M_OTA:
      ota_proc();
      break;
    }
  }
}

// OLED初始化函数
void oled_init()
{
  u8g2.setBusClock(1000000); // 硬件IIC接口使用
  u8g2.begin();
  u8g2.setContrast(255);
  buf_ptr = u8g2.getBufferPtr();
  buf_len = 8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth();
}

void setup()
{
  Serial.begin(115200);
  EEPROM_read_data();
  ui_init();
  oled_init();
  btn_init();
  if (EEPROM_read_single_data("wifi", "reconfig")) // 若读到1，则进入到配网页面
  {
    reconfig_wifi();
    u8g2.clearBuffer();
    ui_proc();
  }

  BH1750_init();
  timer_init();
  Serial.println("Init done");
}

void loop()
{
  btn_scan();
  ui_proc();
  mqtt_loop();
  if (tim1_IRQ_count > 3 && BH1750.retry_cnt < 3)
  {
    BH1750_read();
    if (ui.param[AUTO_BRI])
    {
      ui.param[DISP_BRI] = BH1750.value;
      u8g2.setContrast(ui.param[DISP_BRI]);
    }
    tim1_IRQ_count = 0;
  }
  else
  {
    tim1_IRQ_count = 0;
  }
}
