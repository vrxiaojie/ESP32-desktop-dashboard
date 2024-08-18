#ifndef BH1750_H_
#define BH1750_H_
#include <SlowSoftI2CMaster.h> //使用软件I2C

#define I2C_7BITADDR 0x23
#define MEMLOC 0x10
#define ADDRLEN 1
void BH1750_init(void);
void BH1750_read(void);

struct EnvLightSensor
{
    uint16_t value;
    uint8_t retry_cnt;
};
extern EnvLightSensor BH1750;

#endif