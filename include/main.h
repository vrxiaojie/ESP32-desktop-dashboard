#ifndef MAIN_H_
#define MAIN_H_

#include "Arduino.h"

// 系统版本号
#define SYS_VERSION "V1.0.2"
// UI变量
#define UI_DEPTH 20  // 最深层级数
#define UI_MNUMB 100 // 菜单数量
#define UI_PARAM 20  // 参数数量

/************************************* 定义页面 *************************************/

// 总目录，缩进表示页面层级
enum
{
    M_WINDOW,
    M_SLEEP,
    M_MAIN,
    M_IOT, // 新增IOT管理界面
    M_VOLT,
    M_MONITOR, // 新增监控仪表盘
    M_SETTING,
    M_ABOUT,
    M_OTA, // 新增OTA升级
};

// 状态，初始化标签
enum
{
    S_FADE,      // 转场动画
    S_WINDOW,    // 弹窗初始化
    S_LAYER_IN,  // 层级初始化
    S_LAYER_OUT, // 层级初始化
    S_NONE,      // 直接选择页面
};

// 菜单结构体
typedef struct MENU
{
    const char *m_select;
} M_SELECT;

typedef struct
{
    bool init;
    uint8_t num[UI_MNUMB];
    uint8_t select[UI_DEPTH];
    uint8_t layer;
    uint8_t index = M_SLEEP;
    uint8_t state = S_NONE;
    bool sleep = true;
    uint8_t fade = 1;
    uint8_t param[UI_PARAM];
    double PC_monitor[10];
    uint8_t about[5];
    uint8_t iot[5];
} UI;
extern UI ui;

/************************************* 页面变量 *************************************/
// OLED变量
#define DISP_H 64  // 屏幕高度
#define DISP_W 128 // 屏幕宽度

enum Setting_enum
{
    DISP_BRI,  // 屏幕亮度
    TILE_ANI,  // 磁贴动画速度
    LIST_ANI,  // 列表动画速度
    WIN_ANI,   // 弹窗动画速度
    SPOT_ANI,  // 聚光动画速度
    TAG_ANI,   // 标签动画速度
    FADE_ANI,  // 消失动画速度
    BTN_SPT,   // 按键短按时长
    BTN_LPT,   // 按键长按时长
    CPU_FREQ,  // ESP32 CPU频率
    TILE_UFD,  // 磁贴图标从头展开开关
    LIST_UFD,  // 菜单列表从头展开开关
    TILE_LOOP, // 磁贴图标循环模式开关
    LIST_LOOP, // 菜单列表循环模式开关
    WIN_BOK,   // 弹窗背景虚化开关
    DARK_MODE, // 黑暗模式开关
    AUTO_BRI,  // 自动亮度开关
};

// 磁贴变量
// 所有磁贴页面都使用同一套参数
// #define TILE_B_FONT u8g2_font_xingkai_20 // 磁贴大标题字体
#define TILE_B_FONT u8g2_font_wqy12_t_gb2312a // 磁贴大标题字体 调试时用
#define TILE_B_TITLE_H 18                     // 磁贴大标题字体高度
#define TILE_ICON_H 30                        // 磁贴图标高度
#define TILE_ICON_W 30                        // 磁贴图标宽度
#define TILE_ICON_S 36                        // 磁贴图标间距
#define TILE_INDI_H 27                        // 磁贴大标题指示器高度
#define TILE_INDI_W 7                         // 磁贴大标题指示器宽度
#define TILE_INDI_S 36                        // 磁贴大标题指示器上边距
struct
{
    float title_y_calc = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H * 2;
    float title_y_trg_calc = TILE_INDI_S + (TILE_INDI_H - TILE_B_TITLE_H) / 2 + TILE_B_TITLE_H;
    int16_t temp;
    bool select_flag;
    float icon_x;
    float icon_x_trg;
    float icon_y;
    float icon_y_trg;
    float indi_x;
    float indi_x_trg;
    float title_y;
    float title_y_trg;
} tile;

// 列表变量
// 默认参数

#define LIST_FONT u8g2_font_wqy12_t_gb2312a // 列表字体
#define LIST_TEXT_H 8                       // 列表每行文字字体的高度
#define LIST_LINE_H 16                      // 列表单行高度
#define LIST_TEXT_S 4                       // 列表每行文字的上边距，左边距和右边距，下边距由它和字体高度和行高度决定
#define LIST_BAR_W 3                        // 列表进度条宽度，需要是奇数，因为正中间有1像素宽度的线
#define LIST_BOX_R 0.5f                     // 列表选择框圆角

struct
{
    uint8_t line_n = DISP_H / LIST_LINE_H;
    int16_t temp;
    bool loop;
    float y;
    float y_trg;
    float box_x;
    float box_x_trg;
    float box_y;
    float box_y_trg[UI_DEPTH];
    float bar_y;
    float bar_y_trg;
} list;

// 电压测量页面变量
// 曲线相关
#define WAVE_SAMPLE 20  // 采集倍数
#define WAVE_W 94       // 波形宽度
#define WAVE_L 24       // 波形左边距
#define WAVE_U 0        // 波形上边距
#define WAVE_MAX 27     // 最大值
#define WAVE_MIN 4      // 最小值
#define WAVE_BOX_H 32   // 波形边框高度
#define WAVE_BOX_W 94   // 波形边框宽度
#define WAVE_BOX_L_S 24 // 波形边框左边距
// 列表和文字背景框相关
#define VOLT_FONT u8g2_font_helvB18_tr // 电压数字字体
#define VOLT_TEXT_BG_L_S 24            // 文字背景框左边距
#define VOLT_TEXT_BG_W 94              // 文字背景框宽度
#define VOLT_TEXT_BG_H 29              // 文字背景框高度
struct
{
    int ch0_adc[WAVE_SAMPLE * WAVE_W];
    int ch0_wave[WAVE_W];
    int val;
    float text_bg_r;
    float text_bg_r_trg;
} volt;

// 选择框变量

// 默认参数
#define CHECK_BOX_L_S 95 // 选择框在每行的左边距
#define CHECK_BOX_U_S 2  // 选择框在每行的上边距
#define CHECK_BOX_F_W 12 // 选择框外框宽度
#define CHECK_BOX_F_H 12 // 选择框外框高度
#define CHECK_BOX_D_S 2  // 选择框里面的点距离外框的边距

/*
//超窄行高度测试
#define   CHECK_BOX_L_S       99                          //选择框在每行的左边距
#define   CHECK_BOX_U_S       0                           //选择框在每行的上边距
#define   CHECK_BOX_F_W       5                           //选择框外框宽度
#define   CHECK_BOX_F_H       5                           //选择框外框高度
#define   CHECK_BOX_D_S       1                           //选择框里面的点距离外框的边距
*/
struct
{
    double *d;
    uint8_t *v;
    uint8_t *m;
    uint8_t *s;
    uint8_t *s_p;
} check_box;

// 弹窗变量
#define WIN_FONT u8g2_font_wqy12_t_gb2312a // 弹窗字体
#define WIN_H 32                           // 弹窗高度
#define WIN_W 102                          // 弹窗宽度
#define WIN_BAR_W 92                       // 弹窗进度条宽度
#define WIN_BAR_H 7                        // 弹窗进度条高度
#define WIN_Y -WIN_H - 2                   // 弹窗竖直方向出场起始位置
#define WIN_Y_TRG -WIN_H - 2               // 弹窗竖直方向退场终止位置
struct
{
    // uint8_t
    uint8_t *value;
    uint8_t max;
    uint8_t min;
    uint8_t step;

    MENU *bg;
    uint8_t index;
    char title[20];
    uint8_t select;
    uint8_t l = (DISP_W - WIN_W) / 2;
    uint8_t u = (DISP_H - WIN_H) / 2;
    float bar;
    float bar_trg;
    float y;
    float y_trg;
} win;

// 聚光灯变量
struct
{
    float l;
    float l_trg;
    float r;
    float r_trg;
    float u;
    float u_trg;
    float d;
    float d_trg;
} spot;

/************************************* EEPROM相关定义 *************************************/
struct
{
    bool change;
} eeprom;

void ui_param_init();
void reconfig_wifi_show();

#endif