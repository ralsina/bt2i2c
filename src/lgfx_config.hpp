#include <LovyanGFX.hpp>

// ST7789 240x240 round display configuration
class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;

public:
  LGFX(void)
  {
    { // Bus control settings
      auto cfg = _bus_instance.config();

      // SPI bus settings - ST7789 needs SPI mode 3
      cfg.spi_host = SPI_PORT;   // SPI port (0 or 1)
      cfg.spi_mode = 3;          // SPI mode 3 (CPOL=1, CPHA=1) for ST7789
      cfg.freq_write = 40000000;  // Transmit SPI clock (max 80MHz)
      cfg.freq_read  = 20000000;  // Receive SPI clock

      cfg.pin_sclk = PIN_LCD_SCLK; // SPI SCLK pin
      cfg.pin_mosi = PIN_LCD_MOSI; // SPI MOSI pin
      cfg.pin_miso = -1;           // SPI MISO pin (-1 = disable)
      cfg.pin_dc   = PIN_LCD_DC;   // SPI D/C pin

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { // Display panel control settings
      auto cfg = _panel_instance.config();

      cfg.pin_cs   = PIN_LCD_CS;   // CS pin (-1 = disable)
      cfg.pin_rst  = PIN_LCD_RST;  // RST pin (-1 = disable)
      cfg.pin_busy = -1;           // BUSY pin (-1 = disable)

      cfg.panel_width  = 240;      // Actual display width
      cfg.panel_height = 240;      // Actual display height
      cfg.offset_x     = 0;        // X offset
      cfg.offset_y     = 0;        // Y offset

      cfg.invert = true;           // Invert colors if needed
      // cfg.rgb_order = true;      // Swap R/B if needed

      _panel_instance.config(cfg);
    }

    { // Backlight control settings - use simple GPIO for now
      auto cfg = _light_instance.config();

      cfg.pin_bl   = PIN_LCD_BL;   // Backlight pin
      cfg.invert   = false;         // Don't invert brightness

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};