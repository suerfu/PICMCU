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
    // hardware address on the I2C bus
    char addr[2] = {0xc2, 0xc4};
    char n_addr = 2;
    
    // common instructions to be carried out
    // set maximum blink rate on both rates
    // set PWM on both rates to be MAX -> which is LED is off since LED is driven indirectly by MOSFET
    // 1st driver should set 7th LED driver to use PWM0 -> 10, everything else 00 -> input to LED is high
    // 2st driver should set 5th to use PWM0 -> 10 and 4th LED driver to use PWM1 -> 11, everything else 00 -> input to LED is high
    char instr[2][7] = { {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x80}, {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x0b } };
    char n_instr = 7;
    
    for( char i=0; i<n_addr; i++){
        
        printf("Addressing %x\n\r", addr[i] );
        
        I2C_Master_Start();
        char a = I2C_Master_Write( addr[i] & 0xfe );

        if( a==0 ){
            printf("Device found\n\r");
            
            for( char j=0; j<n_instr; j++){
                char a = I2C_Master_Write( instr[i][j] );
                printf("Writing data %x\n\r", instr[i][j] );
                if( a!=0 ){
                    printf("%d-th instruction of %x not acknowledged\n\r", j, instr[j] );
                }    
            }
        }
        else{
            printf("Device not found at %x\n\r", addr[i]);
        }
        I2C_Master_Stop();
    }
}


// Returns the current LED status hold in the LED driver chip
// index = 0, returns the byte for lower half; = 1 for upper half
//
char ReadLEDStatus( char addr, char index ){
    
    I2C_Master_Start();
    char a = I2C_Master_Write( addr | 0x1 );
            
    if( a != 0 ){
        printf("Device not found at %x\n\r", addr | 0x1 );
        I2C_Master_Stop();
        return 0;
    }
    
    a = I2C_Master_Write( 0x5 + index  );
    if( a!=0 ){
        printf( "Failed to read LED driver register %x\n\r", 0x5+index );
    }

    a = I2C_Master_Read();

    I2C_Master_Stop();
    
    return a;
}

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

#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

