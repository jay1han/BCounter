#include <Esp.h>
#include <Wire.h>
#include "esp_spiffs.h"

// ----------------------------
// I2C for OLED display SSD1306

#define I2C_ADDR     0x3C
#define I2C_GND      3
#define I2C_VCC      2
#define I2C_SCL      1
#define I2C_SDA      0

// Font: only digits

#define FONT_WIDTH 8
#define FONT_PITCH    (FONT_WIDTH + 2)

const unsigned char fontdata_top[][8] = {
    {0xFC,0xFE,0x87,0xC3,0xE3,0x77,0xFE,0xFC,},
    {0x00,0x0C,0x0E,0xFF,0xFF,0x00,0x00,0x00,},
    {0x0C,0x0E,0x07,0x83,0xC3,0xE7,0x7E,0x3C,},
    {0x06,0x07,0x03,0xC3,0xC3,0xE7,0xFE,0x3C,},
    {0xE0,0xF0,0xB8,0x9C,0x8E,0xFF,0xFF,0x80,},
    {0x3F,0x3F,0x33,0x33,0x33,0x73,0xE3,0xC3,},
    {0xF0,0xF8,0xDC,0xCE,0xC7,0xC3,0x83,0x00,},
    {0x03,0x03,0xC3,0xE3,0x73,0x3B,0x1F,0x0F,},
    {0x3C,0x3E,0xE7,0xC3,0xC3,0xE7,0x3E,0x3C,},
    {0x3C,0x7E,0xE7,0xC3,0xC3,0xE7,0xFE,0xFC,},
};

const uint8_t fontdata_bot[][8] = {
    {0x0F,0x1F,0x3B,0x31,0x30,0x38,0x1F,0x0F,},
    {0x00,0x30,0x30,0x3F,0x3F,0x30,0x30,0x00,},
    {0x3C,0x3E,0x37,0x33,0x31,0x30,0x30,0x30,},
    {0x18,0x38,0x30,0x30,0x30,0x39,0x1F,0x0F,},
    {0x01,0x01,0x01,0x01,0x01,0x3F,0x3F,0x01,},
    {0x0C,0x1C,0x38,0x30,0x30,0x38,0x1F,0x0F,},
    {0x0F,0x1F,0x39,0x30,0x30,0x39,0x1F,0x0F,},
    {0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00,},
    {0x0F,0x1F,0x39,0x30,0x30,0x39,0x1F,0x0F,},
    {0x00,0x30,0x30,0x38,0x1C,0x0E,0x07,0x03,},
};

#define LCD_H_RES     72
#define LCD_V_RES     40
#define LCD_PAGES     (LCD_V_RES / 8)
#define LINE_WIDTH    (LCD_H_RES / FONT_PITCH / 2)

// https://www.icbanq.com/icdownload/data/ICBShop/Board/ZJY001_0.42_16P%20OLED.pdf?srsltid=AfmBOoqmlH9ZyInqXiyzBa5Ofm6whEkgQ9E289g3ANLAxYkE7rXdTKrI

const unsigned char ssd1306_init[] = {
    0xAE | 0x00,          // SET_DISP            off
    0xD5, 0xF0,           // SET OSC DIV
    0xA8, LCD_V_RES - 1,  // SET_MUX_RATIO
    0xD3, 0x00,           // SET_DISP_OFFS
    0x40 | 0x00,          // SET_START_LINE
    0x8D, 0x14,           // SET_CHARGE_PUMP     0x10(ext) or 0x14(int)
    0x20, 0x02,           // SET_MEM_ADDRESS     page mode
    0xA0 | 0x01,          // SET_REG_REMAP       horizontal reverse start
    0xC0 | 0x08,          // SET_COM_OUT_DIR     horizontal reverse scan
    0xDA, 0x12,           // SET_COM_PIN_CFG     must be 0x02 if aspect ratio > 2:1, 0x12 otherwise
    0xAD, 0x30,           // SET_IREF           
    0x81, 0x1F,           // SET_CONTRAST
    0xD9, 0x22,           // SET_PRECHARGE
    0xDB, 0x20,           // SET_VCOM_DESEL
    0xA4,                 // SET_ENTIRE_ON
    0xA6 | 0x00,          // SET_NORM_INV (0x01 for inverse)
    0xAE | 0x01,          // SET_DISP            on
};

void ssd1306_cmd(unsigned char cmd) {
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x80);
    Wire.write(cmd);
    Wire.endTransmission();
}

void displayOff() {
    digitalWrite(I2C_VCC, LOW);
    ssd1306_cmd(0xAE);
    ssd1306_cmd(0x00);
}

void sendPage(int page, unsigned char *buffer, int length) {
    ssd1306_cmd(0xB0 | page);
    ssd1306_cmd(0x0C);
    ssd1306_cmd(0x11);
        
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(0x40);
    Wire.write(buffer, length);
    Wire.endTransmission();
}

unsigned char lines[4][LCD_H_RES];
unsigned char bar[LCD_H_RES];

void initDisplay() {
    pinMode(I2C_GND, OUTPUT);
    digitalWrite(I2C_GND, LOW);
    pinMode(I2C_VCC, OUTPUT);
    digitalWrite(I2C_VCC, LOW);
    delay(100);
    digitalWrite(I2C_VCC, HIGH);
    delay(100);
    
    memset(bar, 0, sizeof(bar));
    
    Wire.begin(I2C_SDA, I2C_SCL, 400000);
    Wire.beginTransmission(I2C_ADDR);
    if (Wire.endTransmission() == 0) {
        for (int i = 0; i < sizeof(ssd1306_init); i++) {
            ssd1306_cmd(ssd1306_init[i]);
        }
        delay(100);

        for (int page = 0; page < LCD_PAGES; page ++) {
            sendPage(page, bar, sizeof(bar));
        }
        Serial.println("I2C OLED initialized");
    } else {
        Serial.println("I2C OLED failed");
        esp_deep_sleep_start();
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

void displayNum(char *text) {
    int target = 8;

    for (int i = 0; i < strlen(text) && i < LINE_WIDTH; i++) {
        int index = text[i];
        if (index >= '0' && index <= '9') {
            index -= '0';
            for (int col = 0; col < FONT_WIDTH; col++) {
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
        } else {
            for (int col2 = 0; col2 < FONT_WIDTH * 2; col2++) {
                lines[0][target] = lines[1][target] = lines[2][target] = lines[3][target] = 0x00;
                target++;
            }
        }
        
        for (int space = 0; space < 4; space++) {
            lines[0][target] = lines[1][target] = lines[2][target] = lines[3][target] = 0x00;
            target++;
        }
    }

    for (; target < LCD_H_RES; target++) {
        lines[0][target] = lines[1][target] = lines[2][target] = lines[3][target] = 0x00;
    }
    
    for (int sub = 0; sub < 4; sub++) {
        sendPage(sub, lines[sub], LCD_H_RES);
    }
}

void displayBar(int seconds) {
    if (seconds >= 30) seconds = 30;
    
    int col = 0;
    if (seconds >= 0) {
        for (; col <= seconds; col++) {
            bar[66 - col * 2] = 0x0F;
            bar[67 - col * 2] = 0x0F;
        }
    } 
    for (; col <= 30; col++) {
        bar[66 - col * 2] = 0;
        bar[67 - col * 2] = 0;
    }
    sendPage(4, bar, LCD_H_RES);
}

//-----------------------------
// Storage

esp_vfs_spiffs_conf_t esp_vfs_spiffs_conf = {
    .base_path = "/fs",
    .partition_label = NULL,
    .max_files = 1,
    .format_if_mount_failed = true
};

void initFs() {
    esp_err_t err = esp_vfs_spiffs_register(&esp_vfs_spiffs_conf);
    if (err != ESP_OK) {
        Serial.printf("SPIFFS register => %d\n", err);
        esp_spiffs_format(NULL);
        esp_deep_sleep_start();
    }
}

int readCount() {
    int count = 0;
    FILE *f = fopen("/fs/count", "r");
    if (f == NULL) {
        Serial.println("Error opening file for read");
    } else {
        fscanf(f, "%d", &count);
        fclose(f);
    }
    return count;
}

void setCount(int count) {
    FILE *f = fopen("/fs/count", "w");
    if (f == NULL) {
        Serial.println("Error opening file for write");
    } else {
        fprintf(f, "%d", count);
        fclose(f);
    }
}

// Button

#define PIN_BUTTON   9

bool buttonDown() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUTTON, INPUT);
    initDisplay();
    displayNum("000");
    initFs();
    
    int count = readCount();
    if (count < 999) count++;
    if (count > 999) count = 1;
    setCount(count);

    char num[4];
    sprintf(num, "%3d", count);
    displayNum(num);
    displayBar(60);
}

void loop() {
    static int lastUpdate = 0;
    int seconds = millis() / 1000;
    if (seconds > 30) {
        if (buttonDown()) {
            setCount(0);
            ESP.restart();
        }
        displayOff();
        esp_deep_sleep_start();
        return;
    }
    if (seconds != lastUpdate) displayBar(30 - seconds);
    
    static bool button = false;
    static int pressed = 0;
    
    if (buttonDown()) {
        if (!button) {
            Serial.printf("Button down at %ds\n", seconds);
            button = true;
            pressed = seconds;
        } else {
            if (seconds - pressed > 10) {
                setCount(0);
                ESP.restart();
            }
        }
    } else {
        if (button) {
            Serial.println("Button up");
            button = false;
        }
    }
}
