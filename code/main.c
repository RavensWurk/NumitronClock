/*
 * File:   main.c
 * Author: Raven Minion - Ravenswurk
 *
 * Provides the main logic for the Numitron Clock project. Expects to run on a
 * PIC18F47K40, with I2C to a PCF8583P.
 *
 * Created on 20 March 2019, 19:13
 */


#include "config.h"
#include "i2c1_driver.h"
#include <xc.h>

#define abs(x) (x<0?-x:x)

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t mode;
    uint8_t lastSeconds;
} ClockTime_t;

uint8_t readRegister(unsigned char readReg) {
    uint8_t response = 0;

    i2c1_driver_start();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(0xA0);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(readReg);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_start();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(0xA1);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_startRX();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    response = i2c1_driver_getRXData();

    i2c1_driver_sendNACK();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_stop();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    return response;
}

void writeRegister(unsigned char writeReg, unsigned char value) {
    i2c1_driver_start();
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(0xA0);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(writeReg);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_TXData(value);
    mssp1_waitForEvent();
    mssp1_clearIRQ();

    i2c1_driver_stop();
    mssp1_waitForEvent();
    mssp1_clearIRQ();
}

void updateTime(ClockTime_t* time) {
    time->lastSeconds = time->seconds;
    time->seconds = readRegister(0x02);
    time->minutes = readRegister(0x03);
    uint8_t hours = readRegister(0x04);

    time->mode = (hours & 0x80) >> 7;
    time->hours = hours & 0x7F;
}

void updateIndicator(ClockTime_t clockTime) {
    static uint8_t indicator = 8;

    if (clockTime.lastSeconds != clockTime.seconds) {
        indicator = indicator << 1;

        if (indicator > 8 || indicator == 0) {
            indicator = 1;
        }
    }

    LATD = indicator;
}

inline void blinkStatus(uint8_t leftSide, uint8_t rightSide) {
    for (uint8_t i=0;i<5;i++) {
        LATC=0xFF;
        LATA=0xFF;
        __delay_ms(250);
        LATC = rightSide;
        LATA = leftSide;
        __delay_ms(250);
    }
}

void handleBothButtonsPressed(ClockTime_t clockTime, uint16_t runningTime, uint16_t* buttonHoldTime) {
    if (*buttonHoldTime == -1) {
        *buttonHoldTime = runningTime;
    } else if (abs(runningTime - (*buttonHoldTime)) >= 5) {
        *buttonHoldTime = -1;
        uint8_t leftSide = 0xFF;
        uint8_t rightSide = 0xFF;

        if (clockTime.mode == 0) {
            if (clockTime.hours > 12) {
                writeRegister(0x04, (clockTime.hours - 0x12) | 0x80);
            }

            rightSide = 0x12;
        } else {
            writeRegister(0x04, clockTime.hours & 0x7F);
            leftSide = 0x24;
        }

        blinkStatus(leftSide, rightSide);
    }
}

void handleMinuteButton(ClockTime_t clockTime) {
    clockTime.minutes++;

    if ((clockTime.minutes & 0x0F) >= 10) {
        clockTime.minutes += 0x10;
        clockTime.minutes &= 0xF0;

        if ((clockTime.minutes & 0xF0) >= 0x60) {
            clockTime.minutes = 0;
        }
    }

    LATC = clockTime.minutes;
    writeRegister(0x03, clockTime.minutes);
    __delay_ms(150);
}

void handleHourButton(ClockTime_t clockTime) {
    clockTime.hours++;

    if ((clockTime.hours & 0x0F) >= 10) {
        clockTime.hours += 0x10;
        clockTime.hours &= 0xF0;
    }

    if (clockTime.mode == 0 && ((clockTime.hours & 0xF0) >= 0x20) && ((clockTime.hours & 0x0F) >= 0x04)) {
         clockTime.hours = 0;
    } else if (clockTime.mode == 1 && ((clockTime.hours & 0xF0) >= 0x10) && ((clockTime.hours & 0x0F) >= 0x03)) {
        clockTime.hours = 1;
    }

    LATA = clockTime.hours;
    writeRegister(0x04, clockTime.hours | (clockTime.mode << 7));
    __delay_ms(150);
}

void handleButtons(ClockTime_t clockTime) {
    static uint16_t buttonHoldTime;
    static uint16_t runningTime;
    static uint8_t lastTime;

    // The XC8 compiler doesn't seem to initialise static function variables
    // correctly and just sets them to 0 regardless of what you ask for. Just
    // set them here instead.
    if (buttonHoldTime == 0) {
        buttonHoldTime = -1;
        runningTime = 0;
        lastTime = 0;
    }

    if (clockTime.seconds != lastTime) {
        runningTime++;
        lastTime = clockTime.seconds;
    }

    if (PORTBbits.RB2 && PORTBbits.RB3) {
        handleBothButtonsPressed(clockTime, runningTime, &buttonHoldTime);
    } else {
        buttonHoldTime = -1;

        if (PORTBbits.RB2) {
            handleMinuteButton(clockTime);
        } else if (PORTBbits.RB3) {
            handleHourButton(clockTime);
        }
    }
}

void main(void) {
    TRISA = 0;
    TRISC = 0;
    TRISD = 0;
    LATA = 0;
    LATC = 0;
    PORTD = 0;
    ANSELA = 0;
    ANSELC = 0;
    ANSELB = 0;
    PMD3 = 0xFF;
    PMD4 = 0xEF;

    SSP1CLKPPS = 0x08;   //RB0->MSSP1:SCL1;
    RB1PPS = 0x10;   //RB1->MSSP1:SDA1;
    RB0PPS = 0x0F;   //RB0->MSSP1:SCL1;
    SSP1DATPPS = 0x09;   //RB1->MSSP1:SDA1;

    i2c1_driver_open();
    mssp1_disableIRQ();

    ClockTime_t clockTime = {0};

    while (1) {
        updateTime(&clockTime);

        LATC = clockTime.minutes;
        LATA = clockTime.hours;

        updateIndicator(clockTime);
        handleButtons(clockTime);

        __delay_ms(50);
    }
}
