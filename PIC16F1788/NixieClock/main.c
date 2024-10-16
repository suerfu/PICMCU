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
    InitializeLED();
    
    char curr[3] = {1, 1, 1};
    
    char MAX_PWM = 65;
    
    char targ[3] = { 1, 1, 1};
    for( int i=0; i<3; i++)
        targ[i] = rand() % MAX_PWM;
                
    char equal = 0;
    
    while(1){
        
        while( equal<7 ){
        
            for (int i=0; i<3; i++ ){
                if( curr[i] < targ[i] )
                    curr[i]++;
                else if( curr[i]>targ[i])
                    curr[i]--;
                else{
                    equal |= (0x1 << i);
                }
            }

            SetLEDPWM( curr[0], curr[1], curr[2]);
            ConfigLED( 'r' );
            ConfigLED( 'b' );
            __delay_ms(50);
        }
        printf("Color: %x %x %x\n\r", curr[0], curr[1], curr[2]);
        equal = 0;
        for( int i=0; i<3; i++)
            targ[i] = rand() % MAX_PWM;
        
        /*
        char status[2];
        ReadLEDRegister( 0xc2, status);
        printf("addr:0x%x %x %x\n\r", 0xc2, status[0], status[1]);
        ReadLEDRegister( 0xc4, status);
        printf("addr:0x%x %x %x\n\r", 0xc4, status[0], status[1]);
        for( int i=0; i<10; i++)
        */

    }
    
    return;
}
