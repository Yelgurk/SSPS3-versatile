#pragma once

#ifndef LGFX_SSPS3_V4_hpp
#define LGFX_SSPS3_V4_hpp

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define LCD_D0    18
#define LCD_D1    17
#define LCD_D2    16
#define LCD_D3    15
#define LCD_D4    7
#define LCD_D5    6
#define LCD_D6    5
#define LCD_D7    4
#define LCD_RD    8
#define LCD_WR    19
#define LCD_RS    20
#define LCD_BL    9

class LGFX : public lgfx::LGFX_Device
{
//lgfx::Panel_ILI9481     _panel_instance;
//lgfx::Panel_ILI9486     _panel_instance;

//lgfx::Panel_ILI9488     _panel_instance;
lgfx::Panel_ST7796      _panel_instance;

//lgfx::Panel_ST7789      _panel_instance;

lgfx::Bus_Parallel8 _bus_instance;
//lgfx::Bus_SPI _bus_instance;

lgfx::Light_PWM     _light_instance;

public:
  LGFX()
  {
    {
      auto cfg = _bus_instance.config();

      cfg.freq_write = 32000000;
      cfg.pin_wr = LCD_WR;
      cfg.pin_rd = LCD_RD;
      cfg.pin_rs = LCD_RS;

      cfg.pin_d0  = LCD_D0;
      cfg.pin_d1  = LCD_D1;
      cfg.pin_d2  = LCD_D2;
      cfg.pin_d3  = LCD_D3;
      cfg.pin_d4  = LCD_D4;
      cfg.pin_d5  = LCD_D5;
      cfg.pin_d6  = LCD_D6;
      cfg.pin_d7  = LCD_D7;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.panel_width     = 320;
      cfg.panel_height    = 480;
      cfg.offset_x        = 0;
      cfg.offset_y        = 0;
      cfg.pin_cs          = -1;
      cfg.pin_rst         = -1;
      cfg.pin_busy        = -1;
      cfg.offset_rotation = 3;
      cfg.readable        = false;
      cfg.invert          = true;
      cfg.rgb_order       = false;
      cfg.dlen_16bit      = false;
      cfg.bus_shared      = false;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = LCD_BL;

      _light_instance.config(cfg);
      _panel_instance.light(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

#endif