#include "../configuration.h"
#include "../main.h"
#include <Wire.h>

#ifndef NO_WIRE
uint8_t oled_probe(byte addr)
{
    uint8_t r = 0;
    uint8_t r_prev = 0;
    uint8_t c = 0;
    uint8_t o_probe = 0;
    do {
        r_prev = r;
        Wire.beginTransmission(addr);
        Wire.write(0x00);
        Wire.endTransmission();
        Wire.requestFrom((int)addr, 1);
        if (Wire.available()) {
            r = Wire.read();
        }
        r &= 0x0f;

        if (r == 0x08 || r == 0x00) {
            o_probe = 2; // SH1106
        } else if ( r == 0x03 || r == 0x06 || r == 0x07) {
            o_probe = 1; // SSD1306
        }
        c++;
    } while ((r != r_prev) && (c < 4));
    DEBUG_MSG("0x%x subtype probed in %i tries \n", r, c);
    return o_probe;
}

void scanI2Cdevice(void)
{
    byte err, addr;
    uint8_t r = 0x00;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if (err == 0) {
            DEBUG_MSG("I2C device found at address 0x%x\n", addr);

            nDevices++;

            if (addr == SSD1306_ADDRESS) {
                screen_found = addr;
                screen_model = oled_probe(addr);
                if (screen_model == 1){
                    DEBUG_MSG("ssd1306 display found\n");
                } else if (screen_model == 2){
                    DEBUG_MSG("sh1106 display found\n");
                } else {
                    DEBUG_MSG("unknown display found\n");
                }
            }
#ifdef RV3028_RTC
            if (addr == RV3028_RTC){
                rtc_found = addr;
                DEBUG_MSG("RV3028 RTC found\n");
                Melopero_RV3028 rtc;
                rtc.initI2C();
                rtc.writeToRegister(0x35,0x07); // no Clkout
                rtc.writeToRegister(0x37,0xB4);
            }
#endif
#ifdef PCF8563_RTC
            if (addr == PCF8563_RTC){
                rtc_found = addr;
                DEBUG_MSG("PCF8563 RTC found\n");
            }
#endif
            if (addr == CARDKB_ADDR) {
                cardkb_found = addr;
                // Do we have the RAK14006 instead?
                Wire.beginTransmission(addr);
                Wire.write(0x04); // SENSOR_GET_VERSION
                Wire.endTransmission();
                delay(20);
                Wire.requestFrom((int)addr, 1);
                if (Wire.available()) {
                    r = Wire.read();
                }
                if (r == 0x02) { // KEYPAD_VERSION
                    DEBUG_MSG("RAK14004 found\n");
                    kb_model = 0x02;
                } else {
                    DEBUG_MSG("m5 cardKB found\n");
                    kb_model = 0x00;
                }
            }
            if (addr == FACESKB_ADDR) {
                faceskb_found = addr;
                DEBUG_MSG("m5 Faces found\n");
            }
            if (addr == ST7567_ADDRESS) {
                screen_found = addr;
                DEBUG_MSG("st7567 display found\n");
            }
#ifdef AXP192_SLAVE_ADDRESS
            if (addr == AXP192_SLAVE_ADDRESS) {
                axp192_found = true;
                DEBUG_MSG("axp192 PMU found\n");
            }
#endif
        } else if (err == 4) {
            DEBUG_MSG("Unknow error at address 0x%x\n", addr);
        }
    }

    if (nDevices == 0)
        DEBUG_MSG("No I2C devices found\n");
    else
        DEBUG_MSG("%i I2C devices found\n",nDevices);
}
#else
void scanI2Cdevice(void) {}
#endif
