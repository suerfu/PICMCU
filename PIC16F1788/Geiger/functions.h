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

#include "lcd.h"
    
struct twobytes{
    unsigned char lsb;
    unsigned char msb;
};

union word{
    struct twobytes bytes;
    unsigned int val;
};

unsigned int milisecond = 0;
unsigned int second = 0;
unsigned int minutes = 0;

unsigned int delayMS = 0;

unsigned int radiationCounter = 0;
unsigned int radiationCounter2 = 0;

const char buzzDuration = 1;
char buzzCounter = 0;

char updateHV = 0;
char updateCPM = 0;
unsigned int LCDCount = 0;
unsigned int LCDHV = 0;
unsigned int LCDPHT = 0;

unsigned char countsInterval[6] = {0,0,0,0,0,0,};
unsigned char countsIndex = 0;

#define REFRESH_RATE_HV 2
#define REFRESH_RATE_CPM 60

void Delay( unsigned int a){
    delayMS = a;
    while( delayMS!=0 ){}
    return;
}

void BuzzOnOff( char a ){
    PORTAbits.RA4 = a;
        // TTL output to drive LED/Buzzer
}

char GetBuzzStatus(){
    return PORTAbits.RA4;
        // TTL output to drive LED/Buzzer
}


// IO operations
//
void ConfigPort(){
    TRISA = 0x0f;
    ANSELA = 0x03;
    TRISB = TRISC = 0x0;
    ANSELB = ANSELC = 0x0;
    PORTA = PORTB = PORTC = 0x0;
}


// Clock-related
//
void ConfigClock(){
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    while( (OSCSTAT & 0x58) != 0x58 ){;}
}

void ConfigTimer1(){
    T1CONbits.TMR1CS = 0x0;
        // clock source is instruction clock Fosc/4
    T1CONbits.T1CKPS = 0x3;
        //Fosc = 32 MHz. Timer clock is Focs/4, another pre-scaler by 8 to obtain 1 MHz timer.
    T1GCONbits.TMR1GE = 0;
    T1CONbits.TMR1ON = 1;
}

void Timer1On(){
    T1CONbits.TMR1ON = 1;
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


// Compare module for interrupt
// CCP1 module is used with PWM, so for counter, use CCP2 module.
/*
void ConfigCCP1(){
    CCP1CONbits.CCP1M = 0xb;     // compare with software interrupt
    CCPR1H = 1000 / 256;
    CCPR1L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
}*/

void ConfigCCP2(){
    CCP2CONbits.CCP2M = 0xb;     // compare with software interrupt
    CCPR2H = 1000 / 256;
    CCPR2L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
}


// PWM-related functions
//
void ConfigPWM( unsigned int dc, char period, char prescalar ){
    char high = ( (dc>>2) & 0xff );
    char low = (dc & 0x3);

    // First disable the output
    TRISCbits.TRISC2 = 1;
    
    // Configure timer2 which sets the period
    // Period = (PR2+1) * 4 * (1/Fosc) * TMR2_PreScalarValue
    PR2 = period;
    T2CONbits.T2CKPS = prescalar;
    T2CONbits.TMR2ON = 1;
    
    // Set the resolution
    // PW = CCPRxL:CCPCON<5:4> * (1/Fosc) * TMR2_PreScalarValue
    CCP1CONbits.CCP1M = 0xf;
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


// ADC-related functions
//
void ConfigFVR(){
    FVRCONbits.FVREN = 1;
    FVRCONbits.ADFVR = 0b11;
        // x4 amplification
    while( FVRCONbits.FVRRDY!=1 ){;}
}


void ConfigADC(){
    
    // port configuration
    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSA0 = 1;
        // RA0 is for reading phototransistor
    TRISAbits.TRISA1 = 1;
    ANSELAbits.ANSA1 = 1;
        // RA1 is getting the HV feedback.
    
    // voltage references
    ADCON1bits.ADPREF = 0b11;       // use FVR as input reference
    ADCON1bits.ADNREF = 0;          // use ground as negative reference
    
    // Positive and negative references
    // Should be changed based on applications.
    ADCON0bits.CHS = 0;
        // phototransistor
    ADCON0bits.CHS = 1;
        // HV feedback
    ADCON2bits.CHSN = 0b1111;
    
    // clock
    ADCON1bits.ADCS = 0b010;            // Clock = Fosc/32
    
    // result formatting
    ADCON0bits.ADRMD = 0;           // 12-bit resolution
    ADCON1bits.ADFM = 1;            // two's complement
    
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

// Specific to the Geiger counter module
unsigned int ReadHV(){
    if( ADCON0bits.CHS == 1 ){
        return ReadADC();
    }
    else{
        ADCON0bits.CHS = 1;
        ReadADC();
        return ReadADC();
    }
}


unsigned int ReadBrightness(){
    if( ADCON0bits.CHS == 0 ){
        return ReadADC();
    }
    else{
        ADCON0bits.CHS = 0;
        ReadADC();
        return ReadADC();
    }
}


// Interrupt-related functions
//
void ConfigInterrupt(){   
    INTCON = 0x0;
    
    // ------------- CCP module -------------
    //
    INTCONbits.PEIE = 0x1;
        // enable peripheral
    /*
    PIE1 = 0x0;
    PIE1bits.CCP1IE = 0x1;
        // enable CCP
    */ 
    PIE2 = 0x0;
    PIE2bits.CCP2IE = 0x1;
        // enable CCP
    
    // ------------- IOC module -------------
    // interrupt on change
    INTCONbits.IOCIE = 0x1;
    IOCAP = IOCAN = IOCBP = IOCBN = IOCCP = IOCCN = 0;
    IOCAPbits.IOCAP3 = 0x1;
    
    // enable global interrupt
    INTCONbits.GIE = 0x1;
        // this should be done in the very end
}

void __interrupt() handler(){
    
    //if( PIR1bits.CCP1IF==1 ){
    if( PIR2bits.CCP2IF==1 ){
        milisecond++;
        
        if( buzzCounter>0 ){
            buzzCounter--;
        }
        else{
            BuzzOnOff(0);
        }
        
        if( delayMS>0 ){
            delayMS--;
        }
        
        if( milisecond >= 1000 ){
            milisecond = 0;
            second++;
        }
        
        if( second % REFRESH_RATE_HV==0 && milisecond==0 && updateHV==0 ){
            updateHV = 1;
            LCDHV = ReadHV()/2;
            LCDPHT = ReadBrightness();
            //LCDCount = radiationCounter;
            //LCDCount = countsInterval[0] + countsInterval[1] + countsInterval[2] + countsInterval[3] + countsInterval[4] + countsInterval[5];
            //countsIndex = (countsIndex+1)%6;
            //countsInterval[ countsIndex ] = 0;
        }
        
        if( second >= REFRESH_RATE_CPM ){
            second = 0;
            minutes++;
            
            updateCPM = 1;
            LCDCount = radiationCounter;
            
            printf("%d\n\r", radiationCounter);
            radiationCounter = 0;
        }
        
        //PIR1bits.CCP1IF = 0;
        PIR2bits.CCP2IF = 0;
    }
    
    else if( INTCONbits.IOCIF==1 && IOCAFbits.IOCAF3==1 ){
        
        BuzzOnOff(1);
        buzzCounter = buzzDuration;
        radiationCounter++;
        //countsInterval[ countsIndex ]++;
        
        IOCAFbits.IOCAF3 = 0;
    }
    
    return;
}


// EEPROM memory operations
//
char ReadEEPROM( char addr){
    
    EEADRL = addr;
    
    // access EEPROM
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    
    // initiate read operation and wait for completion
    EECON1bits.RD = 1;
    while( EECON1bits.RD == 1){}
    
    return EEDATL;
}

void WriteEEPROM( char addr, char value){
    
    EEADRL = addr;
    EEDATL = value;
    
    INTCONbits.GIE = 0;
    EECON1bits.WREN = 1;
    
    EECON2 = 0x55;
    EECON2 = 0xaa;
    EECON1bits.WR = 1;
    while( EECON1bits.WR == 1 ){}
    
    EECON1bits.WREN = 0;
    INTCONbits.GIE = 1;

    return;
}

#ifdef	__cplusplus
}
#endif


#endif	/* FUNCTIONS_H */

