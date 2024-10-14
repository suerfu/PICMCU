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
#include "functions.h"

#define _XTAL_FREQ 32000000

void main(void) {
    
    ConfigPort();
    ConfigClock();
    ConfigUSART();
    ConfigI2C();
    
    char LED1[2] = {0x05, 0x55};
    char LED2[2] = {0x06, 0x55};
    
    printf("^^^");
    printf("Welcome!\n\r");
    printf("Welcome!\n\r");
    printf("Welcome!\n\r");
    printf("^^^");
    
    while(1){
        
        for( char i=0; i<0x8; i++){
            printf("Writing data to %x\n\r", 0xc0+2*i );
            I2C_Master_Start();
            I2C_Master_Write( 0xc0+2*i );
            I2C_Master_Write( 0x05 );
            //I2C_Master_Write( 0x55 );
            I2C_Master_Write( 0xff );
            I2C_Master_Stop();
            __delay_ms(1000);
        }
/*
            int a = I2CWrite( 0xc0+i, LED1, 2);
            printf("Writing data to %x\n\r", 0xc0+i );
            
            switch(a){
                case 0:
                    printf("Data received\n\r");
                    break;
                case -1:
                    printf("Device not found\n\r");
                    break;
                case -2:
                    printf("Data not received\n\r");
                    break;
                default:
                    printf("Unknown response %d received\n\r", a);
                    break;
            }
 * */            
            


    }
    
    return;
}

