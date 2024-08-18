#include "BH1750.h"
/********************************** 环境光传感器配置 **********************************/

// 47-->SDA  38-->SCL
SlowSoftI2CMaster si = SlowSoftI2CMaster(47, 38, true);

EnvLightSensor BH1750 = {255, 0};

// 初始化环境光传感器
void BH1750_init()
{
    if (!si.i2c_init())
    {
        Serial.println("I2C init failed");
        BH1750.retry_cnt++;
        return;
    }
    pinMode(47, OUTPUT_OPEN_DRAIN);
    pinMode(38, OUTPUT_OPEN_DRAIN);
}

void BH1750_read()
{
    if (!si.i2c_start((I2C_7BITADDR << 1) | I2C_WRITE))
    {
        Serial.println("I2C device busy");
        return;
    }
    for (byte i = 1; i < ADDRLEN; i++)
        si.i2c_write(0x00);
    si.i2c_write(MEMLOC);
    si.i2c_rep_start((I2C_7BITADDR << 1) | I2C_READ);
    byte val1 = si.i2c_read(false);
    byte val2 = si.i2c_read(true);
    si.i2c_stop();
    BH1750.value = (uint16_t)((val1 * 100 + val2) / 1.2);
    if (BH1750.value > 255)
        BH1750.value = 255;
    else if (BH1750.value < 1)
        BH1750.value = 1;
}