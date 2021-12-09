/* 
 * File:   newmain.c
 * Author: suerfu
 *
 * Created on November 25, 2021, 4:29 PM
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
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

/*
 * 
 */
int main(int argc, char** argv) {
    ConfigPort();
    ConfigClock();
    ConfigUSART();
    
    char type;
    char pot;
    char chan;
    char cmd;
   
    while( 1 ){
        
        type = getch();
        
        if( type=='?' ){
            pot = getch();
            switch( pot ){
                case 'A':
                case 'a':
                    printf("A 0x%x\r\n", PORTA);
                    break;
                case 'B':
                case 'b':
                    printf("B 0x%x\r\n", PORTB);
                    break;
                case 'C':
                case 'c':
                    printf("C 0x%x\r\n", PORTC);
                    break;
                default:
                    break;
            }
        }
        else if( type=='!' ){
            pot = getch();
            switch( pot ){
                case 'A':
                    scanf("%x", &cmd);
                    PORTA = cmd;
                    break;
                case 'a':
                    scanf("%x %x", &chan, &cmd);
                    if( cmd>0 )
                        PORTA |= (1<<chan);
                    else
                        PORTA &= ~(1<<chan);
                    break;
                case 'B':
                    scanf("%x", &cmd);
                    PORTB = cmd;
                    break;
                case 'b':
                    scanf("%x %x", &chan, &cmd);
                    if( cmd>0 )
                        PORTB |= (1<<chan);
                    else
                        PORTB &= ~(1<<chan);
                    break;
                case 'C':
                    scanf("%x", &cmd);
                    PORTC = cmd;
                    break;
                case 'c':
                    scanf("%x %x", &chan, &cmd);
                    if( cmd>0 )
                        PORTC |= (1<<chan);
                    else
                        PORTC &= ~(1<<chan);
                    break;
                default:
                    printf("Port %c not recognized\r\n", pot);
                    break;
            }
        }
        else if( type=='\r' || type=='\n'){
            continue;
        }
        //else{
        //    printf("Command %c not recognized\r\n", type);
        //}
    }

    return 0;
}

