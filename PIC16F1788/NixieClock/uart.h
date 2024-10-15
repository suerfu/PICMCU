/* 
 * File:   uart.h
 * Author: suerfu
 *
 * Created on October 15, 2024, 2:37 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

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

#endif	/* UART_H */

