#pragma once

#if ENABLE_LCD

#include "../system/scheduler.hpp"
#include "font.hpp"
#include "gpio.hpp"
#include "spi.hpp"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "timer.hpp"

#include <stdio.h>

// LCD Control Pins
#define LCD_RS_SET   HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_SET)
#define LCD_RS_RESET HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_RESET)
#define LCD_CS_SET   HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_CS_RESET HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)

// Hardware Configuration
#define SPI_Drv                (&SPI::hspi4)
#define LCD_Brightness_timer   &Timer::htim1
#define LCD_Brightness_channel TIM_CHANNEL_2

// Color Definitions
#define WHITE     0xFFFF
#define BLACK     0x0000
#define BLUE      0x001F
#define BRED      0xF81F
#define GRED      0xFFE0
#define GBLUE     0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define GREEN     0x07E0
#define CYAN      0x7FFF
#define YELLOW    0xFFE0
#define BROWN     0xBC40
#define BRRED     0xFC07
#define GRAY      0x8430
#define DARKBLUE  0x01CF
#define LIGHTBLUE 0x7D7C
#define GRAYBLUE  0x5458

// ST7735 Command Definitions
#define ST7735_NOP                   0x00U
#define ST7735_SW_RESET              0x01U
#define ST7735_READ_ID               0x04U
#define ST7735_READ_STATUS           0x09U
#define ST7735_READ_POWER_MODE       0x0AU
#define ST7735_READ_MADCTL           0x0BU
#define ST7735_READ_PIXEL_FORMAT     0x0CU
#define ST7735_READ_IMAGE_MODE       0x0DU
#define ST7735_READ_SIGNAL_MODE      0x0EU
#define ST7735_SLEEP_IN              0x10U
#define ST7735_SLEEP_OUT             0x11U
#define ST7735_PARTIAL_DISPLAY_ON    0x12U
#define ST7735_NORMAL_DISPLAY_OFF    0x13U
#define ST7735_DISPLAY_INVERSION_OFF 0x20U
#define ST7735_DISPLAY_INVERSION_ON  0x21U
#define ST7735_GAMMA_SET             0x26U
#define ST7735_DISPLAY_OFF           0x28U
#define ST7735_DISPLAY_ON            0x29U
#define ST7735_CASET                 0x2AU
#define ST7735_RASET                 0x2BU
#define ST7735_WRITE_RAM             0x2CU
#define ST7735_RGBSET                0x2DU
#define ST7735_READ_RAM              0x2EU
#define ST7735_PTLAR                 0x30U
#define ST7735_TE_LINE_OFF           0x34U
#define ST7735_TE_LINE_ON            0x35U
#define ST7735_MADCTL                0x36U
#define ST7735_IDLE_MODE_OFF         0x38U
#define ST7735_IDLE_MODE_ON          0x39U
#define ST7735_COLOR_MODE            0x3AU
#define ST7735_FRAME_RATE_CTRL1      0xB1U
#define ST7735_FRAME_RATE_CTRL2      0xB2U
#define ST7735_FRAME_RATE_CTRL3      0xB3U
#define ST7735_FRAME_INVERSION_CTRL  0xB4U
#define ST7735_DISPLAY_SETTING       0xB6U
#define ST7735_PWR_CTRL1             0xC0U
#define ST7735_PWR_CTRL2             0xC1U
#define ST7735_PWR_CTRL3             0xC2U
#define ST7735_PWR_CTRL4             0xC3U
#define ST7735_PWR_CTRL5             0xC4U
#define ST7735_VCOMH_VCOML_CTRL1     0xC5U
#define ST7735_VMOF_CTRL             0xC7U
#define ST7735_WRID2                 0xD1U
#define ST7735_WRID3                 0xD2U
#define ST7735_NV_CTRL1              0xD9U
#define ST7735_READ_ID1              0xDAU
#define ST7735_READ_ID2              0xDBU
#define ST7735_READ_ID3              0xDCU
#define ST7735_NV_CTRL2              0xDEU
#define ST7735_NV_CTRL3              0xDFU
#define ST7735_PV_GAMMA_CTRL         0xE0U
#define ST7735_NV_GAMMA_CTRL         0xE1U
#define ST7735_EXT_CTRL              0xF0U
#define ST7735_PWR_CTRL6             0xFCU
#define ST7735_VCOM4_LEVEL           0xFFU

namespace LCD {
// Display dimensions
constexpr uint8_t HEIGHT = 80;
constexpr uint8_t WIDTH = 160;

// Display control
void init();
void displayOn();
void displayOff();
void setBrightness(uint32_t brightness);
uint32_t getBrightness();
void readID(uint32_t *id);

// Drawing functions
void setCursor(uint8_t x, uint8_t y);
void setPixel(uint8_t x, uint8_t y, uint16_t color);
void drawHLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
void drawVLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color);
void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);
void drawChar(int16_t x, int16_t y, char c, uint8_t size);
void drawString(int16_t x, int16_t y, uint8_t size, char *str);
void update();

// DMA status
extern bool dma_busy;

// Constants
constexpr uint16_t POINT_COLOR = 0xFFFF;
constexpr uint16_t BACK_COLOR = 0x0000;
constexpr uint16_t FRAMEBUFFER_SIZE = WIDTH * HEIGHT * 2; // 2 bytes per pixel

// Buffers
extern uint8_t lcd_data[16];
extern uint8_t framebuffer[FRAMEBUFFER_SIZE];

// Low-level functions
void writeReg(uint8_t reg, uint8_t *data, uint8_t length);
void readReg(uint8_t reg, uint8_t *data);
void sendData(uint8_t *data, uint8_t length);
void recvData(uint8_t *data, uint8_t length);
void waitForDMA();
void setDisplayWindow(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void fillRGBRect(uint8_t x, uint8_t y, uint8_t *data, uint8_t width, uint8_t height);
} // namespace LCD

#endif
