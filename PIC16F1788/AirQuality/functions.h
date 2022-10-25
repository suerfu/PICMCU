/* 
 * File:   functions.h
 * Author: suerfu
 *
 * Created on October 24, 2022, 5:04 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// IO operations
//
void ConfigPort(){
    //TRISA = 0x0f;
    //ANSELA = 0x03;
    //TRISB = TRISC = 0x0;
    //ANSELB = ANSELC = 0x0;
    TRISA = TRISB = TRISC = 0x1;
//    PORTA = PORTB = PORTC = 0x0;
}

#define LED1_LAT LATAbits.LATA5
#define LED1 PORTAbits.RA5
#define LED1_TRIS TRISAbits.TRISA5

#define LED2_LAT LATAbits.LATA6
#define LED2 PORTAbits.RA6
#define LED2_TRIS TRISAbits.TRISA6

#define LED3_LAT LATAbits.LATA7
#define LED3 PORTAbits.RA7
#define LED3_TRIS TRISAbits.TRISA7

#define LED4_LAT LATCbits.LATC0
#define LED4 PORTCbits.RC0
#define LED4_TRIS TRISCbits.TRISC0

void ConfigLEDIndicator(){
    LED1_TRIS = LED2_TRIS = LED3_TRIS = LED4_TRIS = 1;    
    LED1_LAT = 0;
    LED2_LAT = 0;
    LED3_LAT = 0;
    LED4_LAT = 0;
}

void SetVOCIndicator( char level ){
    
    ConfigLEDIndicator();
    
    switch( level ){
        case 1:
            LED1_TRIS = 1;
            LED2_TRIS = 1;
            LED3_TRIS = 0;
            LED4_TRIS = 0;
            LED3 = 1;
            LED4 = 0;
        case 2:
            LED1_TRIS = 0;
            LED2_TRIS = 0;
            LED3_TRIS = 1;
            LED4_TRIS = 1;
            LED1 = 0;
            LED2 = 1;
            
        case 3:
            LED1_TRIS = 0;
            LED2_TRIS = 0;
            LED3_TRIS = 1;
            LED4_TRIS = 1;
            LED1 = 1;
            LED2 = 0;            
        default:
            return;
    }
}

// Clock-related
//
void ConfigClock(){
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    while( (OSCSTAT & 0x58) != 0x58 ){;}
}

// Serial Interface
// Configured for 9600 8N1 mode.
//
void ConfigUSART(){
    //PORTC |= 0xc0;
        // added to fix the issue of USB bridge cannot pull RX pin of the MCU completely high.
        // note: this could be a hardware bug.
    TRISCbits.TRISC7 = 1;  // pin 7 for input
    TRISCbits.TRISC6 = 0;   // pin 6 for output
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
    while(!PIR1bits.RCIF){;}
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

