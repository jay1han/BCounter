#include <Esp.h>
#include <Wire.h>

#include "fontdata.h"

// ----------------------------
// I2C for OLED display SSD1306

#define I2C_ADDR      0x3C
#define I2C_SCL      3
#define I2C_SDA      5

#define FONT_WIDTH    FONT_WIDTH_16
#define FONT_PITCH    (FONT_WIDTH + 1)
#define LCD_H_RES     72
#define LCD_V_RES     40
#define LCD_PAGES     (LCD_V_RES / 8)
#define LINE_WIDTH    (LCD_H_RES / FONT_PITCH / 2)
#define LCD_H_OFFSET  6

const unsigned char ssd1306_init[] = {
    0xAE | 0x00,          // SET_DISP            off
    0xA0 | 0x01,          // SET_REG_REMAP       horizontal reverse start
    0xA8, LCD_V_RES - 1,  // SET_MUX_RATIO
    0xC0 | 0x08,          // SET_COM_OUT_DIR     horizontal reverse scan
    0xDA, 0x12,           // SET_COM_PIN_CFG     must be 0x02 if aspect ratio > 2:1, 0x12 otherwise
    0x20, 0x02,           // SET_MEM_ADDRESS     page mode
    0xD9, 0xF1,           // SET_PRECHARGE
    0xDB, 0x30,           // SET_VCOM_DESEL
    0x81, 0xFF,           // SET_CONTRAST
    0xA4,                 // SET_ENTIRE_ON
    0xA6 | 0x00,          // SET_NORM_INV (0x01 for inverse)
    0x8D, 0x14,           // SET_CHARGE_PUMP
    0xAE | 0x01,          // SET_DISP            on
};

void ssd1306_cmd(unsigned char cmd) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x80);
    Wire.write(cmd);
    Wire.endTransmission();
}

void sendPage(int page, unsigned char *buffer, int length) {
    ssd1306_cmd(0xB0 | page);
    ssd1306_cmd(0x00 | (LCD_H_OFFSET & 0x0F));
    ssd1306_cmd(0x10 | (LCD_H_OFFSET >> 4));
        
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x40);
    Wire.write(buffer, length);
    Wire.endTransmission();
}

unsigned char init_buffer[LCD_H_RES];
void initDisplay() {
    memset(init_buffer, 0, LCD_H_RES);
    
    Wire.begin(I2C_SDA, I2C_SCL, 400000);
    Wire.beginTransmission(I2C_ADDR);
    if (Wire.endTransmission() == 0) {
        for (int i = 0; i < sizeof(ssd1306_init); i++) {
            ssd1306_cmd(ssd1306_init[i]);
        }

        for (int page = 0; page < LCD_PAGES; page ++) {
            sendPage(page, init_buffer, sizeof(init_buffer));
        }
        Serial.println("I2C OLED initialized");
    } else {
        Serial.println("I2C OLED failed");
    }
}

byte nybble_low(byte original) {
    byte result = 0;
    for (int pos = 0; pos < 4; pos++) {
        if (original & (1 << pos)) result |= (3 << (pos * 2));
    }
    return result;
}

byte nybble_hi(byte original) {
    byte result = 0;
    for (int pos = 0; pos < 4; pos++) {
        if (original & (1 << (4 + pos))) result |= (3 << (pos * 2));
    }
    return result;
}

unsigned char lines[4][LCD_H_RES];

void write(char *text) {
    int target = 0;

    for (int i = 0; i < strlen(text) && i < LINE_WIDTH; i++) {
        int index = text[i];
        for (int col = 0; col < FONT_WIDTH_16; col++) {
            lines[0][target] = nybble_low(fontdata_top[index][col]);
            lines[1][target] = nybble_hi(fontdata_top[index][col]);
            lines[2][target] = nybble_low(fontdata_bot[index][col]);
            lines[3][target] = nybble_hi(fontdata_bot[index][col]);
            target++;
            lines[0][target] = nybble_low(fontdata_top[index][col]);
            lines[1][target] = nybble_hi(fontdata_top[index][col]);
            lines[2][target] = nybble_low(fontdata_bot[index][col]);
            lines[3][target] = nybble_hi(fontdata_bot[index][col]);
            target++;
        }
        for (int j = 0; j < 4; j++) {
            lines[0][target] = lines[1][target] = lines[2][target] = lines[3][target] = 0x00;
            target++;
        }
    }
    
    for (int sub = 0; sub < 4; sub++) {
        sendPage(sub, lines[sub], target);
    }
}

void setup() {
}

void loop() {
}
