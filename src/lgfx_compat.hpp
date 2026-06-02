#pragma once

#include <LovyanGFX.hpp>

// Use LovyanGFX but bypass its SPI initialization
// We'll use the working init from display_old.c and just use LGFX for drawing

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void) {
    // Don't init SPI here - we'll do it manually
    // Just set up the panel for drawing
    _panel_instance.setBus(&_bus_instance);
    setPanel(&_panel_instance);
  }

  // Manual initialization using working code
  void init_with_working_sequence(void);
};