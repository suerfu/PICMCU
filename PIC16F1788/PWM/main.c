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

int main(int argc, char** argv) {
    
    ConfigPort();
    //ConfigUSART();
    
    //ConfigPWM( 0x00, 0x0, 0xff, 4);
    //ConfigPSMC1();
    
    char PRH = 0x0f;
    char PRL = 0xff;
    
    char c = 0xff;
    char d = 0xff;
    char dir = 0;
    char inc = 0x0f;
    PORTCbits.RC4 = 0;
    ConfigPSMC1( PRH, PRL, c, d, 0, 0);

    while(1){
        
        //ConfigPWM( c, 0x0, 0xff, 4);
        SetPSMC1( PRH, PRL, c, d, 0, 0);
        
        //for( int i=0; i<0x30; i++){
        //    for( int j=0; j<0xf; j++){}
        //}
        
        if( dir!=0 ){
            if( d==0xff ){
                d = 0;
                c += 1;
            }
            else{
                d += inc;
            }
        }
        else{
            if( d==0 ){
                d = 0xff;
                c -= 1;
            }
            else{
                d -= inc;
            }
        }
        if( c>=PRH && d>=PRL ){
            dir = 0;
        }
        if( c==0x0 && d==0x0 ){
            dir = 1;
        }
        PORTCbits.RC4 = dir;
    }

    return 0;
}
