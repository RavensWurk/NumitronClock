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

void updateIndicator(uint8_t currentSeconds) {
    static uint8_t indicator = 8;
    static uint8_t lastSeconds = 0;

    if (lastSeconds != currentSeconds) {
        lastSeconds = currentSeconds;
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

void handleBothButtonsPressed(uint8_t seconds, uint8_t hours, uint8_t* maxHoursA, uint8_t* maxHoursB, int8_t* buttonHoldTime) {
    if (*buttonHoldTime == -1) {
        *buttonHoldTime = seconds;
    } else if (abs(seconds - (*buttonHoldTime)) >= 5) {
        *buttonHoldTime = -1;
        uint8_t leftSide = 0xFF;
        uint8_t rightSide = 0xFF;

        if (*maxHoursA == 0x20) {
            *maxHoursA = 0x10;
            *maxHoursB = 3;

            if (hours > 12) {
                writeRegister(0x04, hours - 0x12);
            }

            rightSide = 0x12;
        } else {
            *maxHoursA = 0x20;
            *maxHoursB = 4;

            leftSide = 0x24;
        }

        blinkStatus(leftSide, rightSide);
    }
}

void handleMinuteButton(uint8_t minutes) {
    minutes++;

    if ((minutes & 0x0F) >= 10) {
        minutes += 0x10;
        minutes &= 0xF0;

        if ((minutes & 0xF0) >= 0x60) {
            minutes = 0;
        }
    }

    LATC = minutes;
    writeRegister(0x03, minutes);
    __delay_ms(150);
}

void handleHourButton(uint8_t hours, uint8_t maxHoursA, uint8_t maxHoursB) {
    hours++;

    if ((hours & 0x0F) >= 10) {
        hours += 0x10;
        hours &= 0xF0;
    }

    if (((hours & 0xF0) >= maxHoursA) && ((hours & 0x0F) >= maxHoursB)) {
        hours = 0;
    }

    LATA = hours;
    writeRegister(0x04, hours);
    __delay_ms(150);
}

void handleButtons(uint8_t seconds, uint8_t minutes, uint8_t hours) {
    static uint8_t maxHoursA;
    static uint8_t maxHoursB;
    static int8_t buttonHoldTime;

    // The XC8 compiler doesn't seem to initialise static function variables
    // correctly and just sets them to 0 regardless fo what you ask for. Just
    // set them here instead.
    if (maxHoursA == 0) {
        maxHoursA = 0x20;
        maxHoursB = 4;
        buttonHoldTime = -1;
    }

    if (PORTBbits.RB2 && PORTBbits.RB3) {
        handleBothButtonsPressed(seconds, hours, &maxHoursA, &maxHoursB, &buttonHoldTime);
    } else {
        buttonHoldTime = -1;

        if (PORTBbits.RB2) {
            handleMinuteButton(minutes);
        } else if (PORTBbits.RB3) {
            handleHourButton(hours, maxHoursA, maxHoursB);
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

    while (1) {
        uint8_t seconds = readRegister(0x02);
        uint8_t minutes = readRegister(0x03);
        uint8_t hours = readRegister(0x04);
        LATC = minutes;
        LATA = hours;

        updateIndicator(seconds);
        __delay_ms(50);

        handleButtons(seconds, minutes, hours);
    }
}
