<<<<<<< HEAD:PIC16F1788/NixieClock/functions.h
/* Author: suerfu
 *
 * Created on October 14, 2024, 4:54 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// Set default status of some ports that won't be separately configured
//
void ConfigPort(){
    
    // Port B, 0-5 Nixie digits, 6 - RTC input, 7 - HV shutdown signal (low to enable)
    TRISB = 0x40;
    PORTBbits.RB7 = 1;
    ANSELB = 0;   // disable analog functions on 
    
    // Port A, 0-2 for analog input, 3 for digital output, 4-7 for digital input
    TRISA = 0xf7;    // only port3 is for digital output
    PORTAbits.RA3 = 0; // PortA-3 = power pin for analog sensors
    
    // Port C will be separately configured
}

// Configure clock for 32 MHz operation
//
void ConfigClock(){
    
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    
    while( (OSCSTAT & 0x90) != 0x90 ){;}
        // Test if stable
        // OSCSTAT is: T1OSC, PLL, OST-START-UP, HF-ready, HF-locked, MF-ready, LF-ready, HF-stable
}

// Configure port and bits for I2C
//
void ConfigI2C(){
    
    // Configure port for IO
    //
    TRISCbits.TRISC3 = 1;   // input
    TRISCbits.TRISC4 = 1;
    ANSELCbits.ANSC3 = 0;   // disable analog functions
    ANSELCbits.ANSC4 = 0;

    // Configure mode for I2C
    //
    PIR1bits.SSP1IF = 0;
    //SSPCON1bits.SSPM = 0x8;
    //SSPCON1bits.SSPEN = 1;
    SSPCON1 = 0x28;
    SSPCON2 = 0;
    SSPSTAT = 0;
    
    // Configure baud rate
    //
    SSP1ADD = 0x4f; // for 100 kHz operation
    //SSP1ADD = 0x13; // 400 kHz operation
}


void I2C_Master_Wait(){
    while( (SSPSTAT & 0x04) || (SSPCON2 & 0x1F) );
}


void I2C_Master_Start(){
    // set start condition
    I2C_Master_Wait();
    SSPCON2bits.SEN = 1;
}

void I2C_Master_Write( char d ){
    I2C_Master_Wait();
    SSPBUF = d;
}

void I2C_Master_Stop(){
    I2C_Master_Wait();
    SSPCON2bits.PEN = 1;
}

/*
int I2CWaitAck(){

    for( char i=0; i<255; i++){
        if( SSPCON2bits.ACKSTAT==0 ){
            return 0;
        }
    }
    return -1;  // slave not present/data not received
}
*/

int I2CWrite( char addr, char* data, char n){
    
    I2C_Master_Start();
    
    //while( SSPCON2bits.SEN==1 );
    //I2CIFPoll();

    // write address and write-bit
    I2C_Master_Wait();
    SSP1BUF = ( addr & 0xfe );
    //while( SSPSTATbits.BF == 1 ){}

    // check ack status
    //int ack = I2CWaitAck();
    //if( ack<0 ){
    //    return -1;  // slave not present
    //}
    
    // write data for transmission
    for( int i=0; i<n; i++){
        I2C_Master_Wait();
        SSP1BUF = data[i];
        /*while( SSPSTATbits.BF == 1 ){}
        ack = I2CWaitAck();
        if( ack<0 ){
            return -2;  // slave present but data not received
        }*/
    }

    return 0;
}



void ConfigUSART(){
    
    // Configue Port - disable analog and configure for TX output
    TRISCbits.TRISC6 = 0;   // pin 6 for output
    ANSELCbits.ANSC6 = 0;

    // Configure TX register
    //
    TXSTA = 0x24; // 2 for TXEN, 4 for BRGH (high baud rate)
  
    // Configue Port - disable analog and configure for RX output
    //
    TRISCbits.TRISC7 = 1;   // pin 7 for input
    ANSELCbits.ANSC7 = 0;   // disable analog functions
    
    // Configure RX register
    //
    RCSTA |= 0x90; // 9 for SPEN and CREN (receiver enable)
    
    // Set baud rate for 9600 by default
    //
    BAUDCON = 0x08; // use 16-bit baud rate generator
    //SPBRGH = 832/256;
    //SPBRGL = 832%256; // 9600
    SPBRGH = 0; 
    SPBRGL = 68; // 115200
}


void putch(char c){
    while(!PIR1bits.TXIF){;}
    TXREG = c;
}


int getch(void){
    while(!PIR1bits.RCIF){}
    return RC1REG;
}


int getche(void){
    char c = getch();
    putch(c);
    return c;
}

#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */


=======
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

>>>>>>> nixieclock:PIC16F1788/Projects/NixieClock/functions.h
