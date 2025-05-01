#include "lcd.hpp"

static uint8_t lcd_data[16];

void LCD::writeReg(uint8_t reg, uint8_t* data, uint8_t length) {
  LCD_CS_RESET;  // Ensure CS is low before starting
  LCD_RS_RESET;
  HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
  LCD_RS_SET;
  if (length > 0) {
    HAL_SPI_Transmit(SPI_Drv, data, length, 500);
  }
  LCD_CS_SET;  // Release CS after transmission
}

void LCD::readReg(uint8_t reg, uint8_t* data) {
  LCD_CS_RESET;  // Ensure CS is low before starting
  LCD_RS_RESET;
  HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
  LCD_RS_SET;
  HAL_SPI_Receive(SPI_Drv, data, 1, 500);
  LCD_CS_SET;  // Release CS after transmission
}

void LCD::sendData(uint8_t* data, uint8_t length) {
  LCD_CS_RESET;  // Ensure CS is low before starting
  HAL_SPI_Transmit(SPI_Drv, data, length, 500);
  LCD_CS_SET;  // Release CS after transmission
}

void LCD::recvData(uint8_t* data, uint8_t length) {
  LCD_CS_RESET;  // Ensure CS is low before starting
  HAL_SPI_Receive(SPI_Drv, data, length, 500);
  LCD_CS_SET;  // Release CS after transmission
}

void LCD::init() {
  // Initialize CS and RS pins
  LCD_CS_SET;
  LCD_RS_SET;
  Scheduler::yieldDelay(100);  // Initial delay for power stabilization

  // Initialize Brightness Timer
  HAL_TIMEx_PWMN_Start(LCD_Brightness_timer, LCD_Brightness_channel);
  SetBrightness(100);  // Set initial brightness

  // Software Reset
  writeReg(ST7735_SW_RESET, nullptr, 0);
  Scheduler::yieldDelay(150);  // Increased delay after reset
  writeReg(ST7735_SW_RESET, nullptr, 0);
  Scheduler::yieldDelay(150);  // Increased delay after reset

  // Sleep out
  lcd_data[0] = 0;
  writeReg(ST7735_SLEEP_OUT, lcd_data, 1);
  Scheduler::yieldDelay(150);  // Wait for sleep out to complete

  // Frame rate ctl, normal mode
  lcd_data[0] = 0x01; lcd_data[1] = 0x2C; lcd_data[2] = 0x2D;
  writeReg(ST7735_FRAME_RATE_CTRL1, lcd_data, 3);

  // Frame rate ctl, idle mode
  lcd_data[0] = 0x01; lcd_data[1] = 0x2C; lcd_data[2] = 0x2D;
  writeReg(ST7735_FRAME_RATE_CTRL2, lcd_data, 3);

  // Frame rate ctl, partial mode
  lcd_data[0] = 0x01; lcd_data[1] = 0x2C; lcd_data[2] = 0x2D;
  lcd_data[3] = 0x01; lcd_data[4] = 0x2C; lcd_data[5] = 0x2D;
  writeReg(ST7735_FRAME_RATE_CTRL3, lcd_data, 6);

  // Display inversion ctl
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
  lcd_data[0] = 0x05;  // 16-bit color
  writeReg(ST7735_COLOR_MODE, lcd_data, 1);

  // Memory access control - RGB order, normal orientation
  // MADCTL bits:
  // MY: Row address order (0: top to bottom, 1: bottom to top)
  // MX: Column address order (0: left to right, 1: right to left)
  // MV: Row/Column exchange (0: normal, 1: exchange)
  // ML: Vertical refresh order (0: top to bottom, 1: bottom to top)
  // RGB: RGB/BGR order (0: RGB, 1: BGR)
  // MH: Horizontal refresh order (0: left to right, 1: right to left)
  lcd_data[0] = 0xA0;  // Landscape mode rotated 180 degrees (MY=1, MX=1, MV=1)
  writeReg(ST7735_MADCTL, lcd_data, 1);

  // Display inversion on
  writeReg(ST7735_DISPLAY_INVERSION_ON, nullptr, 0);

  // Set display window to full screen
  // Column address set (CASET)
  lcd_data[0] = 0; lcd_data[1] = 0; lcd_data[2] = 0; lcd_data[3] = WIDTH - 1;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  // Row address set (RASET)
  lcd_data[0] = 0; lcd_data[1] = 0; lcd_data[2] = 0; lcd_data[3] = HEIGHT - 1;
  writeReg(ST7735_RASET, lcd_data, 4);

  // Main screen on
  lcd_data[0] = 0x00;
  writeReg(ST7735_DISPLAY_ON, lcd_data, 1);
  Scheduler::yieldDelay(150);  // Wait for display to turn on
}

void LCD::setDisplayWindow(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  // Add calibration offsets for 0.96" ST7735 display in landscape mode rotated 180 degrees
  // Using HannStar panel offsets (Xpos += 1, Ypos += 26)
  x += 1;
  y += 26;
  
  // Column address set (CASET)
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x + width - 1;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  // Row address set (RASET)
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

void LCD::SetBrightness(uint32_t brightness) {
  __HAL_TIM_SetCompare(LCD_Brightness_timer, LCD_Brightness_channel, brightness);
}

uint32_t LCD::getBrightness() {
  return __HAL_TIM_GetCompare(LCD_Brightness_timer, LCD_Brightness_channel);
}

void LCD::setCursor(uint8_t x, uint8_t y) {
  // Set column address (CASET)
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  // Set row address (RASET)
  lcd_data[0] = 0; lcd_data[1] = y; lcd_data[2] = 0; lcd_data[3] = y;
  writeReg(ST7735_RASET, lcd_data, 4);
  
  // Start writing to RAM
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
}

void LCD::fillRGBRect(uint8_t x, uint8_t y, uint8_t* data, uint8_t width, uint8_t height) {
  if ((x + width) > WIDTH || (y + height) > HEIGHT) return;
  
  // Set display window for the entire rectangle
  setDisplayWindow(x, y, width, height);
  
  // Start writing to RAM
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
  
  // Send color data for each pixel
  for (uint16_t i = 0; i < (uint16_t)width * height; i++) {
    uint16_t color = ((uint16_t*)data)[i];
    uint8_t color_data[2];
    color_data[0] = color & 0xFF;  // LSB first
    color_data[1] = color >> 8;    // MSB second
    sendData(color_data, 2);
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
      temp = ascii_1206[c][t];  // Use 12x6 font
    } else {
      temp = ascii_1608[c][t];  // Use 16x8 font
    }

    // Process each bit in the byte
    for (uint8_t t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80) {
        uint8_t col = t / 2;  // Each pair of bytes represents a column
        uint8_t row = t1 + ((t % 2) * 8);  // First byte of pair is top 8 rows, second byte is bottom 4/8 rows
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
  color = (uint16_t)((uint16_t)color << 8);
  color |= (uint16_t)((uint16_t)(color >> 8));

  if (x >= WIDTH || y >= HEIGHT) return;

  setCursor(x, y);
  sendData((uint8_t*)&color, 2);
}

void LCD::drawHLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
  if ((x + length) > WIDTH) return;
  
  // Set column address (CASET)
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x + length - 1;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  // Set row address (RASET)
  lcd_data[0] = 0; lcd_data[1] = y; lcd_data[2] = 0; lcd_data[3] = y;
  writeReg(ST7735_RASET, lcd_data, 4);
  
  // Start writing to RAM
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
  
  // Prepare color data
  uint8_t color_data[2];
  color_data[0] = color >> 8;
  color_data[1] = color & 0xFF;
  
  // Send color data for each pixel
  for (uint8_t i = 0; i < length; i++) {
    sendData(color_data, 2);
  }
}

void LCD::drawVLine(uint8_t x, uint8_t y, uint8_t length, uint16_t color) {
  if ((y + length) > HEIGHT) return;
  
  // Set column address (CASET)
  lcd_data[0] = 0; lcd_data[1] = x; lcd_data[2] = 0; lcd_data[3] = x;
  writeReg(ST7735_CASET, lcd_data, 4);
  
  // Set row address (RASET)
  lcd_data[0] = 0; lcd_data[1] = y; lcd_data[2] = 0; lcd_data[3] = y + length - 1;
  writeReg(ST7735_RASET, lcd_data, 4);
  
  // Start writing to RAM
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
  
  // Prepare color data
  uint8_t color_data[2];
  color_data[0] = color >> 8;
  color_data[1] = color & 0xFF;
  
  // Send color data for each pixel
  for (uint8_t i = 0; i < length; i++) {
    sendData(color_data, 2);
  }
}

void LCD::fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color) {
  if ((x + width) > WIDTH || (y + height) > HEIGHT) return;
  
  // Set display window
  setDisplayWindow(x, y, width, height);
  
  // Start writing to RAM
  writeReg(ST7735_WRITE_RAM, nullptr, 0);
  
  // Prepare color data - swap bytes for RGB565
  uint8_t color_data[2];
  color_data[0] = color & 0xFF;  // LSB first
  color_data[1] = color >> 8;    // MSB second
  
  // Send color data for each pixel
  for (uint16_t i = 0; i < (uint16_t)width * height; i++) {
    sendData(color_data, 2);
  }
}