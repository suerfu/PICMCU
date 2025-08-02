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


// UART
// Baud rate control (BAUDCON): SYNC, BRGH, BRG16, SPBRGH-SPGRGL controls the baud rate.
    
//  TXSTA: Transmit status and control - CSRC | TX9 | TXEN | SYNC | SENDB | BRGH | TRMT | TX9D
//      CSRC: clock source select, doesn't care in asynchronous mode
//      TXEN: bit 5, set 1 to enable, 0 to disable
//      SYNC: set 0 for asynchronous, set 1 for synchronous
//      SENDB: send break character, under async mode, 1 = send sync break on next transmission, 0 = sync break transmission compete
//      BRGH: high baud rate select bit, 1 = high speed, 0 = low-speed
//      TRMT: transmit shift register status bit, 1 = empty, 0 = full, this bit is read-only
//      TX9D: 9th bit of transmit data
//  Data transmission by writing TXREG register.
//
//  RXSTA: Receiver status: SPEN | RX9 | SREN | CREN | ADDEN | FERR | OERR | RX9D
//      SPEN: serial port enable, 1 = (enables EUSART and configures TX pin), 0 = disable
//          * analog function must be disabled.
//          * user must manually set TRIS bit to configure RX pin.
//      RX9: 1 = receive 9th bit, 0 = 8-bit reception
//      SREN: single receive enable, don't care in asynchronous mode
//      CREN: continuous receive enable bit, under asynchronous mode, 1 = enable receiver, 0 = disable receiver
//      ADDEN: enable address detection, in async 8-bit mode, doesn't care
//      FERR, OERR, RX9D: framing, ordering error and 9th bit in 9-bit mode, all read-only.
//  Receive the data by reading RCREG register
//
//  BAUDCON: Baud rate generator, ABDOVF | RCIDL | - | SCKP | BRG16 | - | WUE | ABDEN
//      BRG16: use 16-bit baud rate generator
//
//  SPBRGH, SPBRGL: used to determine the actual baud rate. For the actual value, refer to the table
//
//
//
// Below code configures MCU for 9600 8-bit, 1 stop bit, no parity check and no traffic control.
//
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
    SPBRGH = 832/256;
    SPBRGL = 832%256; // for these numbers, refer to table 28-5 in data sheet

    // use below code for 115200 baud rate
    /*
    SPBRGH = 0;
    SPBRGL = 68;    // 115200
    */
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
