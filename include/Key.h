#ifndef KEY_H_
#define KEY_H_

#include "Arduino.h"
#include "main.h"

// 可按下旋钮引脚
#define BTN0 14     //UP
#define BTN1 12     //DOWN
#define BTN2 13     //确认

// 按键ID
#define BTN_ID_CC 0 // 逆时针旋转
#define BTN_ID_CW 1 // 顺时针旋转
#define BTN_ID_SP 2 // 短按
#define BTN_ID_LP 3 // 长按

// 按键变量
#define BTN_PARAM_TIMES 2 // 由于uint8_t最大值可能不够，但它存储起来方便，这里放大两倍使用

typedef struct
{
    bool val;
    bool last_val;
    long count;
    unsigned long chg_time;
    bool operated;
} KEY;

// 按键信息
typedef struct
{
    uint8_t id;
    bool pressed;
} Key_Status;
;

extern KEY key[3];
extern Key_Status volatile btn;

bool get_key_val(uint8_t ch);
void btn_scan(void);
void btn_init(void);

#endif