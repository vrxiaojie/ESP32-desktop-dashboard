#ifndef PREF_EEPROM_H_
#define PREF_EEPROM_H_

#include "Preferences.h"
#include "main.h"

void EEPROM_write_data(void);
void EEPROM_read_data(void);
void EEPROM_write_single_data(const char* part_name,const char* name, uint32_t value); //写入一组数据
uint32_t EEPROM_read_single_data(const char* part_name,const char* name); //读取一组数据

#endif