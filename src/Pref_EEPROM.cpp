#include "Pref_EEPROM.h"

/********************************** EEPROM断电保存配置 使用Preferences库********************************/
void EEPROM_write_data()
{
  Preferences prefs; // 声明Preferences对象
  prefs.begin("ui");
  for (uint8_t i = 0; i < UI_PARAM; i++)
  {
    switch (i)
    {
    case DISP_BRI:
      // 若为自动亮度则不写入亮度值
      if (prefs.getUInt("DISP_BRI", ui.param[DISP_BRI]) != ui.param[DISP_BRI] && !ui.param[AUTO_BRI])
      {
        prefs.putUInt("DISP_BRI", ui.param[DISP_BRI]);
      }
      break;
    case TILE_ANI:
      if (prefs.getUInt("TILE_ANI", ui.param[TILE_ANI]) != ui.param[TILE_ANI])
      {
        prefs.putUInt("TILE_ANI", ui.param[TILE_ANI]);
      }
      break;
    case LIST_ANI:
      if (prefs.getUInt("LIST_ANI", ui.param[LIST_ANI]) != ui.param[LIST_ANI])
      {
        prefs.putUInt("LIST_ANI", ui.param[LIST_ANI]);
        Serial.println("LIST_ANI write");
      }
      break;
    case WIN_ANI:
      if (prefs.getUInt("WIN_ANI", ui.param[WIN_ANI]) != ui.param[WIN_ANI])
      {
        prefs.putUInt("WIN_ANI", ui.param[WIN_ANI]);
      }
      break;
    case SPOT_ANI:
      if (prefs.getUInt("SPOT_ANI", ui.param[SPOT_ANI]) != ui.param[SPOT_ANI])
      {
        prefs.putUInt("SPOT_ANI", ui.param[SPOT_ANI]);
      }
      break;
    case TAG_ANI:
      if (prefs.getUInt("TAG_ANI", ui.param[TAG_ANI]) != ui.param[TAG_ANI])
      {
        prefs.putUInt("TAG_ANI", ui.param[TAG_ANI]);
      }
      break;
    case FADE_ANI:
      if (prefs.getUInt("FADE_ANI", ui.param[FADE_ANI]) != ui.param[FADE_ANI])
      {
        prefs.putUInt("FADE_ANI", ui.param[FADE_ANI]);
      }
      break;
    case BTN_SPT:
      if (prefs.getUInt("BTN_SPT", ui.param[BTN_SPT]) != ui.param[BTN_SPT])
      {
        prefs.putUInt("BTN_SPT", ui.param[BTN_SPT]);
      }
      break;
    case BTN_LPT:
      if (prefs.getUInt("BTN_LPT", ui.param[BTN_LPT]) != ui.param[BTN_LPT])
      {
        prefs.putUInt("BTN_LPT", ui.param[BTN_LPT]);
      }
      break;
    case CPU_FREQ:
      if (prefs.getUInt("CPU_FREQ", ui.param[CPU_FREQ]) != ui.param[CPU_FREQ])
      {
        prefs.putUInt("CPU_FREQ", ui.param[CPU_FREQ]);
      }
      break;
    case TILE_UFD:
      if (prefs.getUInt("TILE_UFD", ui.param[TILE_UFD]) != ui.param[TILE_UFD])
      {
        prefs.putUInt("TILE_UFD", ui.param[TILE_UFD]);
      }
      break;
    case LIST_UFD:
      if (prefs.getUInt("LIST_UFD", ui.param[LIST_UFD]) != ui.param[LIST_UFD])
      {
        prefs.putUInt("LIST_UFD", ui.param[LIST_UFD]);
      }
      break;
    case TILE_LOOP:
      if (prefs.getUInt("TILE_LOOP", ui.param[TILE_LOOP]) != ui.param[TILE_LOOP])
      {
        prefs.putUInt("TILE_LOOP", ui.param[TILE_LOOP]);
      }
      break;
    case LIST_LOOP:
      if (prefs.getUInt("LIST_LOOP", ui.param[LIST_LOOP]) != ui.param[LIST_LOOP])
      {
        prefs.putUInt("LIST_LOOP", ui.param[LIST_LOOP]);
      }
      break;
    case WIN_BOK:
      if (prefs.getUInt("WIN_BOK", ui.param[WIN_BOK]) != ui.param[WIN_BOK])
      {
        prefs.putUInt("WIN_BOK", ui.param[WIN_BOK]);
      }
      break;
    case DARK_MODE:
      if (prefs.getUInt("DARK_MODE", ui.param[DARK_MODE]) != ui.param[DARK_MODE])
      {
        prefs.putUInt("DARK_MODE", ui.param[DARK_MODE]);
      }
      break;
    case AUTO_BRI:
      if (prefs.getUInt("AUTO_BRI", ui.param[AUTO_BRI]) != ui.param[AUTO_BRI])
      {
        prefs.putUInt("AUTO_BRI", ui.param[AUTO_BRI]);
      }
      break;
    }
  }
  prefs.end();
}

void EEPROM_write_single_data(const char *part_name, const char *name, uint32_t value) // 写入一组数据
{
  Preferences prefs; // 声明Preferences对象
  prefs.begin(part_name);
  prefs.putUInt(name, value);
  prefs.end();
}

uint32_t EEPROM_read_single_data(const char *part_name, const char *name) // 读取一组数据
{
  Preferences prefs; // 声明Preferences对象
  prefs.begin(part_name);
  uint32_t val = prefs.getUInt(name);
  prefs.end();
  return val;
}

void EEPROM_read_data()
{
  Preferences prefs; // 声明Preferences对象
  prefs.begin("ui");
  ui_param_init();
  for (uint8_t i = 0; i < UI_PARAM; i++)
  {
    switch (i)
    {
    case DISP_BRI:
      ui.param[DISP_BRI] = prefs.getUInt("DISP_BRI", ui.param[DISP_BRI]);
      break;
    case TILE_ANI:
      ui.param[TILE_ANI] = prefs.getUInt("TILE_ANI", ui.param[TILE_ANI]);
      Serial.println(ui.param[TILE_ANI]);
      break;
    case LIST_ANI:
      ui.param[LIST_ANI] = prefs.getUInt("LIST_ANI", ui.param[LIST_ANI]);
      break;
    case WIN_ANI:
      ui.param[WIN_ANI] = prefs.getUInt("WIN_ANI", ui.param[WIN_ANI]);
      break;
    case SPOT_ANI:
      ui.param[SPOT_ANI] = prefs.getUInt("SPOT_ANI", ui.param[SPOT_ANI]);
      break;
    case TAG_ANI:
      ui.param[TAG_ANI] = prefs.getUInt("TAG_ANI", ui.param[TAG_ANI]);
      break;
    case FADE_ANI:
      ui.param[FADE_ANI] = prefs.getUInt("FADE_ANI", ui.param[FADE_ANI]);
      break;
    case BTN_SPT:
      ui.param[BTN_SPT] = prefs.getUInt("BTN_SPT", ui.param[BTN_SPT]);
      break;
    case BTN_LPT:
      ui.param[BTN_LPT] = prefs.getUInt("BTN_LPT", ui.param[BTN_LPT]);
      break;
    case CPU_FREQ:
      ui.param[CPU_FREQ] = prefs.getUInt("CPU_FREQ", ui.param[CPU_FREQ]);
      break;
    case TILE_UFD:
      ui.param[TILE_UFD] = prefs.getUInt("TILE_UFD", ui.param[TILE_UFD]);
      break;
    case LIST_UFD:
      ui.param[LIST_UFD] = prefs.getUInt("LIST_UFD", ui.param[LIST_UFD]);
      break;
    case TILE_LOOP:
      ui.param[TILE_LOOP] = prefs.getUInt("TILE_LOOP", ui.param[TILE_LOOP]);
      break;
    case LIST_LOOP:
      ui.param[LIST_LOOP] = prefs.getUInt("LIST_LOOP", ui.param[LIST_LOOP]);
      break;
    case WIN_BOK:
      ui.param[WIN_BOK] = prefs.getUInt("WIN_BOK", ui.param[WIN_BOK]);
      break;
    case DARK_MODE:
      ui.param[DARK_MODE] = prefs.getUInt("DARK_MODE", ui.param[DARK_MODE]);
      break;
    case AUTO_BRI:
      ui.param[AUTO_BRI] = prefs.getUInt("AUTO_BRI", ui.param[AUTO_BRI]);
      break;
    default:
      break;
    }
  }
  prefs.end();
}
