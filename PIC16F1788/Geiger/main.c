// PIC16F1788 Configuration Bit Settings
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


#include <xc.h>

#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

int main(int argc, char** argv) {

    ConfigPort();
    ConfigClock();
    
    ConfigUSART();
    
    ConfigFVR();
    ConfigADC();
    
    union word period;
    period.val = 200;
    
    union word duty_cycle;
    duty_cycle.val = 10;
    //char PRH = 0x0f;
    //char PRL = 0xff;
        // period high and low
    //char DCH = 0x0;
    //char DCL = 0x0;
        // duty cycle high and low
    
    ConfigPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0 );
    
    char command[8];
        // used to obtain command from user
    
    printf("Welcome!\n\r");
    
    while(1){
    
        scanf( "%s", command );
        
        if( strncmp(command, "!pr", 3)==0 ){
            scanf( "%u", &period.val );
            SetPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0);
            //printf("Changing period to %x %x\n\r", PRH, PRL);
        }
        else if( strncmp(command, "!dc", 3)==0 ){
            scanf( "%u", &duty_cycle.val );
            SetPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0);
            //printf("Changing duty cycle to %x %x\n\r", DCH, DCL);
        }
        else if( strncmp(command, "?pr", 3)==0 ){
            printf("%u\n\r", period.val );
        }
        else if( strncmp(command, "?dc", 3)==0 ){
            printf("%u\n\r", duty_cycle.val );
        }
        else if( strncmp(command, "?hv", 3)==0 ){
            printf("%u\n\r", ReadHV() );
        }
    }

    return 0;
}
