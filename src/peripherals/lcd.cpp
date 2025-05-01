#include "lcd.hpp"

// Static member initialization
uint8_t LCD::lcd_data[16];
uint8_t LCD::framebuffer[FRAMEBUFFER_SIZE];
bool LCD::dma_busy = false;

void LCD::writeReg(uint8_t reg, uint8_t* data, uint8_t length) {
  waitForDMA();
  LCD_CS_RESET;
  LCD_RS_RESET;
  HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
  LCD_RS_SET;
  if (length > 0) {
    HAL_SPI_Transmit(SPI_Drv, data, length, 500);
  }
  LCD_CS_SET;
}

void LCD::readReg(uint8_t reg, uint8_t* data) {
  waitForDMA();
  LCD_CS_RESET;
  LCD_RS_RESET;
  HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
  LCD_RS_SET;
  HAL_SPI_Receive(SPI_Drv, data, 1, 500);
  LCD_CS_SET;
}

void LCD::sendData(uint8_t* data, uint8_t length) {
  waitForDMA();
  LCD_CS_RESET;
  HAL_SPI_Transmit(SPI_Drv, data, length, 500);
  LCD_CS_SET;
}

void LCD::recvData(uint8_t* data, uint8_t length) {
  waitForDMA();
  LCD_CS_RESET;
  HAL_SPI_Receive(SPI_Drv, data, length, 500);
  LCD_CS_SET;
}

void LCD::waitForDMA() {
  while (dma_busy) {
    Scheduler::yield();
  }
}

void LCD::init() {
  // Initialize CS and RS pins
  LCD_CS_SET;
  LCD_RS_SET;
  Scheduler::yieldDelay(100);

  // Initialize Brightness Timer
  HAL_TIMEx_PWMN_Start(LCD_Brightness_timer, LCD_Brightness_channel);
  setBrightness(100);

  // Software Reset
  writeReg(ST7735_SW_RESET, nullptr, 0);
  Scheduler::yieldDelay(150);
  writeReg(ST7735_SW_RESET, nullptr, 0);
  Scheduler::yieldDelay(150);

  // Sleep out
  lcd_data[0] = 0;
  writeReg(ST7735_SLEEP_OUT, lcd_data, 1);
  Scheduler::yieldDelay(150);

  // Frame rate control
  lcd_data[0] = 0x01; lcd_data[1] = 0x2C; lcd_data[2] = 0x2D;
  writeReg(ST7735_FRAME_RATE_CTRL1, lcd_data, 3);
  writeReg(ST7735_FRAME_RATE_CTRL2, lcd_data, 3);
  writeReg(ST7735_FRAME_RATE_CTRL3, lcd_data, 6);

  // Display inversion control
  lcd_data[0] = 0x07;
  writeReg(ST7735_FRAME_INVERSION_CTRL, lcd_data, 1);

  // Power control
  lcd_data[0] = 0xA2; lcd_data[1] = 0x02; lcd_data[2] = 0x84;
  writeReg(ST7735_PWR_CTRL1, lcd_data, 3);
  lcd_data[0] = 0xC5;
  writeReg(ST7735_PWR_CTRL2, lcd_data, 1);
  lcd_data[0] = 0x0A; lcd_data[1] = 0x00;
  writeReg(ST7735_PWR_CTRL3, lcd_data, 2);
  lcd_data[0] = 0x8A; lcd_data[1] = 0x2A;
  writeReg(ST7735_PWR_CTRL4, lcd_data, 2);
  lcd_data[0] = 0x8A; lcd_data[1] = 0xEE;
  writeReg(ST7735_PWR_CTRL5, lcd_data, 2);

  // VCOM control
  lcd_data[0] = 0x0E;
  writeReg(ST7735_VCOMH_VCOML_CTRL1, lcd_data, 1);

  // Inversion off
  writeReg(ST7735_DISPLAY_INVERSION_OFF, nullptr, 0);

  // Color mode - 16-bit RGB565
  lcd_data[0] = 0x05;
  writeReg(ST7735_COLOR_MODE, lcd_data, 1);

  // Memory access control - RGB order, normal orientation
  lcd_data[0] = 0xA0;  // Landscape mode rotated 180 degrees
  writeReg(ST7735_MADCTL, lcd_data, 1);

  // Display inversion on
  writeReg(ST7735_DISPLAY_INVERSION_ON, nullptr, 0);

  // Set display window to full screen
  lcd_data[0] = 0; lcd_data[1] = 0; lcd_data[2] = 0; lcd_data[3] = WIDTH - 1;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  lcd_data[0] = 0; lcd_data[1] = 0; lcd_data[2] = 0; lcd_data[3] = HEIGHT - 1;
  writeReg(ST7735_RASET, lcd_data, 4);

  // Main screen on
  lcd_data[0] = 0x00;
  writeReg(ST7735_DISPLAY_ON, lcd_data, 1);
  Scheduler::yieldDelay(150);

  // Clear framebuffer
  for (uint16_t i = 0; i < FRAMEBUFFER_SIZE; i += 2) {
    framebuffer[i] = 0x00;
    framebuffer[i + 1] = 0x00;
  }
}

void LCD::setDisplayWindow(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  // Add calibration offsets for 0.96" ST7735 display in landscape mode rotated 180 degrees
  x += 1;
  y += 26;
  
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x + width - 1;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  lcd_data[0] = 0; lcd_data[1] = y; lcd_data[2] = 0; lcd_data[3] = y + height - 1;
  writeReg(ST7735_RASET, lcd_data, 4);
}

void LCD::readID(uint32_t* id) {
  uint8_t tmp[3];
  readReg(ST7735_READ_ID1, &tmp[0]);
  readReg(ST7735_READ_ID2, &tmp[1]);
  readReg(ST7735_READ_ID3, &tmp[2]);

  *id = ((uint32_t)tmp[2]) | ((uint32_t)tmp[1])<<8 | ((uint32_t)tmp[0])<<16;
}

void LCD::displayOn() {
  lcd_data[0] = 0x00;
  writeReg(ST7735_NORMAL_DISPLAY_OFF, lcd_data, 1);
  writeReg(ST7735_DISPLAY_ON, lcd_data, 1);
  writeReg(ST7735_MADCTL, lcd_data, 1);
}

void LCD::displayOff() {
  lcd_data[0] = 0x00;
  writeReg(ST7735_NORMAL_DISPLAY_OFF, lcd_data, 1);
  writeReg(ST7735_DISPLAY_OFF, lcd_data, 1);
  writeReg(ST7735_MADCTL, lcd_data, 1);
}

void LCD::setBrightness(uint32_t brightness) {
  __HAL_TIM_SetCompare(LCD_Brightness_timer, LCD_Brightness_channel, brightness);
}

uint32_t LCD::getBrightness() {
  return __HAL_TIM_GetCompare(LCD_Brightness_timer, LCD_Brightness_channel);
}

void LCD::setCursor(uint8_t x, uint8_t y) {
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  lcd_data[0] = 0; lcd_data[1] = y; lcd_data[2] = 0; lcd_data[3] = y;
  writeReg(ST7735_RASET, lcd_data, 4);
  
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
}

void LCD::fillRGBRect(uint8_t x, uint8_t y, uint8_t* data, uint8_t width, uint8_t height) {
  if ((x + width) > WIDTH || (y + height) > HEIGHT) return;
  
  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      uint16_t pixel = ((uint16_t*)data)[row * width + col];
      uint16_t fb_index = ((y + row) * WIDTH + (x + col)) * 2;
      framebuffer[fb_index] = pixel & 0xFF;
      framebuffer[fb_index + 1] = pixel >> 8;
    }
  }
}

void LCD::drawChar(uint16_t x, uint16_t y, char c, uint8_t size) {
  const uint8_t font_width = (size == 12 ? 6 : 8);
  const uint8_t font_height = size;
  uint16_t write[font_height][font_width];
  uint8_t temp, y0 = y, x0 = x;

  // Initialize write array
  for (uint8_t i = 0; i < font_height; i++) {
    for (uint8_t j = 0; j < font_width; j++) {
      write[i][j] = 0;
    }
  }

  c = c - ' ';  // Get offset value

  // Process each byte in the font data
  for (uint8_t t = 0; t < size; t++) {
    if (size == 12) {
      temp = ascii_1206[c][t];
    } else {
      temp = ascii_1608[c][t];
    }

    // Process each bit in the byte
    for (uint8_t t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80) {
        uint8_t col = t / 2;
        uint8_t row = t1 + ((t % 2) * 8);
        if (row < font_height) {
          write[row][col] = POINT_COLOR;
        }
      }
      temp <<= 1;
    }
  }

  fillRGBRect(x0, y0, (uint8_t *)&write, font_width, font_height);
}

void LCD::drawString(uint8_t x, uint8_t y, uint8_t size, char* str) {
  while (*str != '\0') {
    drawChar(x, y, *str, size);
    x += (size == 12 ? 6 : 8);
    str++;
  }
}

void LCD::setPixel(uint8_t x, uint8_t y, uint16_t color) {
  if (x >= WIDTH || y >= HEIGHT) return;

  uint16_t fb_index = (y * WIDTH + x) * 2;
  framebuffer[fb_index] = color & 0xFF;
  framebuffer[fb_index + 1] = color >> 8;
}

void LCD::drawHLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
  if ((x + length) > WIDTH) return;
  
  for (uint8_t i = 0; i < length; i++) {
    setPixel(x + i, y, color);
  }
}

void LCD::drawVLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
  if ((y + length) > HEIGHT) return;
  
  for (uint8_t i = 0; i < length; i++) {
    setPixel(x, y + i, color);
  }
}

void LCD::fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color) {
  if ((x + width) > WIDTH || (y + height) > HEIGHT) return;
  
  for (uint8_t row = 0; row < height; row++) {
    for (uint8_t col = 0; col < width; col++) {
      setPixel(x + col, y + row, color);
    }
  }
}

void LCD::update() {
  waitForDMA();
  
  setDisplayWindow(0, 0, WIDTH, HEIGHT);
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
  
  LCD_CS_RESET;
  dma_busy = true;
  HAL_SPI_Transmit_DMA(SPI_Drv, framebuffer, FRAMEBUFFER_SIZE);
}