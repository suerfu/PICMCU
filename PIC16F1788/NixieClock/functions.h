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

