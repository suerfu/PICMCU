/* 
 * File:   functions.h
 * Author: suerfu
 *
 * Created on November 25, 2021, 4:35 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

void ConfigPort(){
    TRISA = TRISB = TRISC = 0;
    ANSELA = ANSELB = ANSELC = 0;
    PORTA = PORTB = PORTC = 0x0;
}

void ConfigClock(){
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    while( (OSCSTAT & 0x58) != 0x58 ){;}
}

void ConfigUSART(){
    TRISCbits.TRISC7 = 1;  // pin 7 for input
    TRISCbits.TRISC6 = 0;   // pin 6 for output
    //ANSELCbits.ANSC7 = 0;
    //ANSELCbits.ANSC6 = 0;
    TXSTA = 0x24;
    RCSTA |= 0x90; // 90 for enabling continuous receiving
    BAUDCON = 0x08;
    SPBRGH = 832/256;
    SPBRGL = 832%256;
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

