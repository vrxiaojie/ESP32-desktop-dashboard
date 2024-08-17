#include "Key.h"
/************************************* 旋钮相关 *************************************/

// 按键变量

KEY key[3] = {false};
Key_Status volatile btn;

bool get_key_val(uint8_t ch)
{
    switch (ch)
    {
    case 0:
        return digitalRead(BTN0);
        break;
    case 1:
        return digitalRead(BTN1);
        break;
    case 2:
        return digitalRead(BTN2);
        break;
    default:
        break;
    }
    return false;
}

void btn_scan()
{
    for (uint8_t i = 0; i < (sizeof(key) / sizeof(KEY)); ++i)
    {
        key[i].val = get_key_val(i);       // 获取键值
        if (key[i].last_val != key[i].val) // 发生改变
        {
            key[i].last_val = key[i].val; // 更新状态
            // delay(ui.param[BTN_SPT]);
            // key[i].val = get_key_val(i);
            if (key[i].val == HIGH)
            {
                key[i].chg_time = millis();
                switch (i)
                {
                case 0:
                    btn.id = BTN_ID_CC;
                    btn.pressed = 1;
                    break;
                case 1:
                    btn.id = BTN_ID_CW;
                    btn.pressed = 1;
                    break;
                default:
                    break;
                }
            }
            if (key[i].val == LOW)
            {
                switch (i)
                {
                case 2:
                    if (key[i].operated == 1)
                        key[i].operated = 0;
                    else
                    {
                        btn.id = BTN_ID_SP;
                        btn.pressed = 1;
                    }
                    break;
                default:
                    break;
                }
            }
        }
        if (key[i].val == HIGH && millis() - key[i].chg_time > ui.param[BTN_LPT] && key[i].operated == 0)
        {
            switch (i)
            {
            case 0:
                btn.id = BTN_ID_CC;
                btn.pressed = 1;
                break;
            case 1:
                btn.id = BTN_ID_CW;
                btn.pressed = 1;
                break;
            case 2:
                btn.id = BTN_ID_LP;
                key[i].operated = 1;
                btn.pressed = 1;
                break;
            default:
                break;
            }
        }
    }
}

// 初始化按键
void btn_init()
{
    pinMode(BTN0, INPUT_PULLDOWN);
    pinMode(BTN1, INPUT_PULLDOWN);
    pinMode(BTN2, INPUT_PULLDOWN);
    for (uint8_t i = 0; i < (sizeof(key) / sizeof(KEY)); ++i)
    {
        key[i].val = key[i].last_val = get_key_val(i);
    }
}