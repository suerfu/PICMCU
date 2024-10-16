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


// Configure RGB LED - initial state
//
void ConfigLED(){    
    
    // common instructions to be carried out
    // set maximum blink rate on both rates
    // set PWM on both rates to be MAX -> which is LED is off since LED is driven indirectly by MOSFET
    // 1st driver should set 7th LED driver to use PWM0 -> 10, everything else 00 -> input to LED is high
    // 2st driver should set 5th to use PWM0 -> 10 and 4th LED driver to use PWM1 -> 11, everything else 00 -> input to LED is high
    
    char addr[] = {0xc2, 0xc4};
    char n_addr = sizeof(addr)/sizeof(addr[0]);

    char instr[2][7] = { {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, /*0x80*/0x40}, {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, /*0x0b*/0x05 } };
    char n_instr = sizeof(instr[0])/sizeof(instr[0][0]);
        // default state all of them off

    printf("Initializing RGB LEDs...\n\r");
    
    for( char i=0; i<n_addr; i++){
        printf("Addressing %x\n\r", addr[i] );
        I2C_Write( addr[i], instr[i], n_instr);
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
/*
// Configure RGB LED
//
void ConfigRLED( char pwm, char power ){
        
    // hardware address on the I2C bus
    char addr = 0xc4;
    
    // Get current LED status
    char stat = ReadLEDStatus( addr, 1);
    printf("Current R LED status: %x\n\r", stat);
    
    if( power==0 )
        stat = ( (stat & 0xf3) + 0x4 );
            // turn red led off, which is xx xx 01 xx
    else{
        stat = ( (stat & 0xf3) + 0x8 );
            // use PWM setting 0, which is xx xx 10 xx
        pwm = 0xff - pwm;
    }
    char instr[4] = { 0x02, pwm, 0x06, stat };
    char n_instr = sizeof(instr)/sizeof(instr[0]);
    
    printf("Addressing %x\n\r", addr );
        
    I2C_Master_Start();
    char a = I2C_Master_Write( addr );
    if( a != 0 ){
        printf("Device not found at %x\n\r", addr);
        I2C_Master_Stop();
        return;
    }
        
    for( char j=0; j<n_instr; j++){
        char a = I2C_Master_Write( instr[j] );
        printf("Writing data %x\n\r", instr[j] );
        if( a!=0 ){
            printf("%d-th Instruction of %x not acknowledged\n\r", j, instr[j] );
        }
    }
        
    I2C_Master_Stop();
}
*/
#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

