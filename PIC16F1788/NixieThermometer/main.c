/* 
 * File:   main.c
 * Author: suerfu
 *
 * Created on December 27, 2022, 5:16 PM
 */

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

#include <stdio.h>
#include <stdlib.h>

#include <xc.h>

#include "functions.h"

/*
 * 
 */

extern char print_output;

int main(int argc, char** argv) {

    ConfigPort();
    
    ConfigClock();
    
    ConfigUSART();
    
    ConfigADC();  
    
    ConfigTimer1();
    Timer1On();
    ConfigCCP1();
    
//    ConfigTimer0();
    
    ConfigInterrupt();
   
    printf("Welcome!\n\r");
    
    SensorPower(1);
    
    char command[16];
        // character string to receive inputs
    
    char dc_temperature;
        // duty cycle for temperature PWM
    char dc_humidity;
        // duty cycle for humidity PWM
    
    while(1){
//        if ( print_output==1 ){
//            printf("%f %f %f\r\n", ReadTemperature(), ReadHumidity(), ReadPositiveReference() );
            //printf("%u %u %u\r\n", ReadADCChannel(0), ReadADCChannel(1), ReadADCChannel(3) );
//            print_output = 0;
//        }
        if( PIR1bits.RCIF!=0 ){
            
            scanf( "%s", command );
        
            if( strncmp(command, "!tmp", 4)==0 ){
                scanf( "%hhu", &dc_temperature );
                SetTemperaturePWM( dc_temperature );
            }
            else if( strncmp(command, "!hum", 4)==0 ){
                scanf( "%hhu", &dc_humidity );
                SetHumidityPWM( dc_humidity );
                //WriteEEPROM( 3, duty_cycle.bytes.msb );
            }
        }
    }

    return 0;
}



