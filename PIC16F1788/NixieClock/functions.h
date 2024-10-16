/* 
 * File:   functions.h
 * Author: suerfu
 *
 * Created on October 14, 2024, 4:54 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "mssp.h"

    
// common instructions to be carried out
// set maximum blink rate on both rates
// set PWM on both rates to be MAX -> which is LED is off since LED is driven indirectly by MOSFET
// 1st driver should set 7th LED driver to use PWM0 -> 10, everything else 00 -> input to LED is high
// 2st driver should set 5th to use PWM0 -> 10 and 4th LED driver to use PWM1 -> 11, everything else 00 -> input to LED is high
//
char LED_REGISTER[2][7] = { {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x80/*0x40*/}, {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x0b/*0x05*/ } };
char N_LED_REGISTER = 7;

char LED_ADDR[] = {0xc2, 0xc4};
char N_ADDR = 2;


void ConfigLED( char c);


void InitializeLED(){
    printf("Initializing RGB LEDs...\n\r");
    ConfigLED('r');
        // green LED is on the same register, so no need to separately configure g
    ConfigLED('b');
}


// Configure LED
// This function simply writes the status of LED_Register onto the LED driver by I2C
//
void ConfigLED( char color ){
    switch( color){
        case 'r':
        case 'g':
            I2C_Write( LED_ADDR[1], LED_REGISTER[1], N_LED_REGISTER);
            break;
        case 'b':
            I2C_Write( LED_ADDR[0], LED_REGISTER[0], N_LED_REGISTER);
            break;
        default:
            break;
    }
}


// Returns the current LED status hold in the LED driver chip
// index = 0, returns the byte for lower half; = 1 for upper half
//
void ReadLEDRegister( char addr, char* led ){
    char cmd = 0x15;
    I2C_Read( addr, &cmd, 1, led, 2 );
    return;
}


// Function to set the PWM value of LEDs
// This only affects the LED status variable and does not actually write to device
//
void SetLEDPWM( char r, char g, char b ){
    char pwm_r = 0xff - r;
    char pwm_g = 0xff - g;
    char pwm_b = 0xff - b;
    
    LED_REGISTER[1][2] = pwm_r;
    LED_REGISTER[1][4] = pwm_g;
    LED_REGISTER[0][2] = pwm_b;
}


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

