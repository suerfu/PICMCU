/* 
 * File:   functions.h
 * Author: suerfu
 *
 * Created on July 24, 2022, 2:02 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

struct twobytes{
    unsigned char lsb;
    unsigned char msb;
};

union word{
    struct twobytes bytes;
    unsigned int val;
};
    
void ConfigPort(){
    TRISA = TRISB = TRISC = 0x0;
    ANSELA = ANSELB = ANSELC = 0x0;
    PORTA = PORTB = PORTC = 0x0;
}

void ConfigClock(){
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    while( (OSCSTAT & 0x58) != 0x58 ){;}
}

// Configured for 9600 8N1 mode.
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

void ConfigPWM( char high, char low, char period, char prescalar ){
    // First disable the output
    TRISCbits.TRISC2 = 1;
    
    // Configure timer2 which sets the period
    // Period = (PR2+1) * 4 * (1/Fosc) * TMR2_PreScalarValue
    PR2 = period;
    T2CONbits.T2CKPS = prescalar;
    T2CONbits.TMR2ON = 1;
    
    // Set the resolution
    // PW = CCPRxL:CCPCON<5:4> * Tosc * TMR2_PreScalarValue
    CCP1CONbits.CCP1M = 0xff;
    CCP1CONbits.DC1B = low;
    CCPR1L = high;
    TRISCbits.TRISC2 = 0;
}

void ConfigPSMC1( char PRH, char PRL, char DCH, char DCL, char PHH, char PHL ){

    // Set period
    PSMC1PRH = PRH;
    PSMC1PRL = PRL;

    // Duty cycle
    PSMC1DCH = DCH;
    PSMC1DCL = DCL;//0xff;

    // Phase/start of active pulse
    PSMC1PHH = PHH;
    PSMC1PHL = PHL;
    
    // Set PSMC clock
    PSMC1CLK = 0x00;
        // clock source: 0x?0 => Fosc, 0x?1 => 64 MHz internal, 0x?2 => external
        // pre-scalar:  0x0? => f/1, 0x1? => f/2, 0x2? => f/4, 0x3? => f/8
    
    // Output port, polarity and enable output
    PSMC1STR0 = 0x2;
        // enable output on PSMC1B
    //PSMC1STR1bits.P1SSYNC = 0x0;
        // sync output immediately
        
    PSMC1POLbits.P1POLB = 0;    // 0 for active high
    PSMC1OENbits.P1OEB = 1;    // output enable bit
    
    // Set source of event for period, phase and duty cycle counter
    PSMC1PRS = 0x1;
    PSMC1PHS = 0x1;
    PSMC1DCS = 0x1;
        // LSB = 1 for enabling synchronous event triggered by a match with TMR, disabling all other modes.
    
    // enable steering and load the values, and enable digital driver
    PSMC1CON = 0b11000000;
        // bit7 is enable bit, bit6 is Load, need to clear to enable
        // bit0-3 set to 0 for single PWM output
    TRISCbits.TRISC1 = 0;
}

void SetPSMC1( char PRH, char PRL, char DCH, char DCL, char PHH, char PHL ){
    // Set period
    PSMC1PRH = PRH;
    PSMC1PRL = PRL;
    // Duty cycle
    PSMC1DCH = DCH;
    PSMC1DCL = DCL;//0xff;
    // Phase/start of active pulse
    PSMC1PHH = PHH;
    PSMC1PHL = PHL;
    // Load the values
    PSMC1CON = 0b11000000;
}


void ConfigFVR(){
    FVRCONbits.FVREN = 1;
    FVRCONbits.ADFVR = 0b11;
        // x4 amplification
    while( FVRCONbits.FVRRDY!=1 ){;}
}


void ConfigADC(){
    
    // port configuration
    TRISAbits.TRISA1 = 1;
    ANSELAbits.ANSA1 = 1;
        // RA1 is getting the HV feedback.
    
    // voltage references
    ADCON1bits.ADPREF = 0b11;       // use FVR as input reference
    ADCON1bits.ADNREF = 0;          // use ground as negative reference
    
    // Positive and negative references
    // Should be changed based on applications.
    ADCON0bits.CHS = 1;
    ADCON2bits.CHSN = 0b1111;
    
    // clock
    ADCON1bits.ADCS = 0b010;            // Clock = Fosc/32
    
    // result formatting
    ADCON0bits.ADRMD = 0;           // 12-bit resolution
    ADCON1bits.ADFM = 0;            // sign and magnitude
    
    // enable ADC
    ADCON0bits.ADON = 1;
}


unsigned int ReadADC(){
    ADCON0bits.GO = 1;
        // start ADC    
    union word vol;
        // value to be filled and returned
    while( ADCON0bits.GO == 1 ){;}
        // wait for conversion to be done
    vol.bytes.msb = ADRESH;
    vol.bytes.lsb = ADRESL;
    return vol.val;
}


unsigned int ReadHV(){
    ADCON0bits.CHS = 1;
    return ReadADC();
}


#ifdef	__cplusplus
}
#endif


#endif	/* FUNCTIONS_H */

