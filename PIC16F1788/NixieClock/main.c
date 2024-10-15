/*
 * File:   main.c
 * Author: suerfu
 * Created on October 14, 2024, 4:54 PM
 */

// PIC16F1788 Configuration Bit Settings
// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
#pragma config DEBUG = OFF      // In-Circuit Debugger Mode (In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "clock.h"
#include "uart.h"
#include "mssp.h"

#include "functions.h"

#define _XTAL_FREQ 32000000

void main(void) {
    
    ConfigPort();
    ConfigClock();
    ConfigI2C();
    ConfigUSART();

    __delay_ms(1);
    printf("Configuring LED...\n\r");
    //ConfigLED();
    

    /*
    char pwm = 0;
    while(1){
        printf("Configuring Red LED\n\r");
//        ConfigRLED( pwm, 0);
        pwm++;
        for(int i=0; i<30; i++)
            __delay_ms(1000);
    }
    */
    
    
    char addr[] = {0xc2, 0xc4};
    char n_addr = sizeof(addr)/sizeof(addr[0]);

    //char instr[] = {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x0b};
    //char instr[2][7] = { {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x80}, {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x0b } };
        // to PWM smallest value
    char instr[2][7] = { {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x40}, {0x11, 0x0, 0xff, 0x0, 0xff, 0x00, 0x05 } };
        // all LED off
    char n_instr = sizeof(instr[0])/sizeof(instr[0][0]);

    printf("Welcome!\n\r");
    
    char pwm = 0xff;
    
    while(1){
        
        //instr[2] = pwm;
        //pwm--;
        
        for( char i=0; i<n_addr; i++){
            printf("Addressing %x\n\r", addr[i] );
            
            I2C_Master_Start();
            char a = I2C_Master_Write( addr[i] & 0xfe );
            
            if( a == 0 ){
                for( char j=0; j<n_instr; j++){
                    char a = I2C_Master_Write( instr[i][j] );
                    printf("Writing data %x\n\r", instr[i][j] );
                    if( a!=0 ){
                        printf("%d-th Instruction of %x not acknowledged\n\r", j, instr[i][j] );
                    }
                }
            }
            else{
                printf("Device not found at %x\n\r", addr[i]);
            }
            
            I2C_Master_Stop();
            
            __delay_ms(100);
        }
    }
    

    return;
}
