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
    int val;
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
int LCDCount = 0;
int LCDHV = 0;

unsigned char countsInterval[6] = {0,0,0,0,0,0,};
unsigned char countsIndex = 0;

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
//
void ConfigCCP1(){
    CCP1CONbits.CCP1M = 0xb;     // compare with software interrupt
    CCPR1H = 1000 / 256;
    CCPR1L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
}


// PWM-related functions
//
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


int ReadADC(){
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
int ReadHV(){
    if( ADCON0bits.CHS == 1 ){
        return ReadADC();
    }
    else{
        ADCON0bits.CHS = 1;
        ReadADC();
        return ReadADC();
    }
}


int ReadBrightness(){
    if( ADCON0bits.CHS == 0 ){
        return ReadADC();
    }
    else{
        ADCON0bits.CHS = 0;
        ReadADC();
        return ReadADC();
    }
}
/*
void LCDWrite( char cmd, char mode ){
    PORTAbits.RA6 = mode;
        // select command or data
    //PORTAbits.RA7 = 0;
        // read/~write pin, should always be 0 in this application
    PORTB = cmd;
        // PORTB is the actual data to be sent
    PORTAbits.RA5 = 0;
    PORTAbits.RA5 = 1;
    PORTAbits.RA5 = 0;
        // Pulse the enable pin
    Delay(1);
}

void LCDCommand( char cmd ){
    LCDWrite(cmd, 0);
}

void LCDData( char data ){
    LCDWrite( data, 1);
}

void LCDClear(){
    LCDCommand( LCD_CLEARDISPLAY );
    Delay(2);
}

void LCDHome(){
    LCDCommand( LCD_RETURNHOME );
    Delay(2);
}

#define ROW_INCR 0x40
#define MAX_ROW 2
//#define MAX_COL 0x28
#define MAX_COL 16

void LCDCursor( char row, char col){
    LCDCommand( LCD_SETDDRAMADDR | ( (row%MAX_ROW)*ROW_INCR + col%MAX_COL) );
}

void ConfigLCD(){
    
    PORTAbits.RA4 = 1;
    
    // Configure direction of data flow
    TRISAbits.TRISA5 = 0;
    TRISAbits.TRISA6 = 0;
    TRISAbits.TRISA7 = 0;
    TRISB = 0;
    TRISCbits.TRISC2 = 0;
    
    // Set default values
    PORTAbits.RA7 = 0;
    PORTCbits.RC2 = 1;
    
    // Allow for setup time of about 40 ms
    Delay( 50 );
    PORTAbits.RA6 = 0;  // RS
    PORTAbits.RA5 = 0;  // EN
    
    // Function set
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );
    Delay( 5 );
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );
    Delay( 2 );
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );

    LCDClear();
    
    // Display control
    LCDCommand( LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON );
    
    // Entry mode
    LCDCommand( (LCD_ENTRYMODESET | LCD_ENTRYRIGHT) & ~LCD_ENTRYSHIFTDISPLAY );
    
    // Cursor shift
    // LCDCommand( (LCD_CURSORSHIFT | LCD_MOVERIGHT) & ~LCD_DISPLAYSHIFT );
    
    PORTAbits.RA4 = 0;
}

void LCDPrint( char temp[], char n, char row, char col ){
    LCDCursor( row, col);
    for( char i=0; i<n; i++){
        LCDData( temp[i] );
    }
}

char LCDCountDisplay[6];
char LCDHVDisplay[4];

void UpdateLCD( unsigned int cpm, unsigned int hv){    
    sprintf( LCDCountDisplay, "%-6u", cpm );
    sprintf( LCDHVDisplay, "%-4u", hv );
    LCDPrint( LCDCountDisplay, 6, 0, 8);
    LCDPrint( LCDHVDisplay, 4, 1, 8);
}
*/


// Interrupt-related functions
//
void ConfigInterrupt(){   
    INTCON = 0x0;
    
    // ------------- CCP module -------------
    INTCONbits.PEIE = 0x1;
        // enable peripheral
    PIE1 = 0x0;
    PIE1bits.CCP1IE = 0x1;
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
    
    if( PIR1bits.CCP1IF==1 ){
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
        
        if( second%1==0 && milisecond==0 && updateHV==0 ){
            updateHV = 1;
            LCDHV = ReadHV()/2;
            //LCDCount = radiationCounter;
            //LCDCount = countsInterval[0] + countsInterval[1] + countsInterval[2] + countsInterval[3] + countsInterval[4] + countsInterval[5];
            //countsIndex = (countsIndex+1)%6;
            //countsInterval[ countsIndex ] = 0;
        }
        
        if( second >=60 ){
            second = 0;
            minutes++;
            
            updateCPM = 1;
            LCDCount = radiationCounter;
            
            printf("%d\n\r", radiationCounter);
            radiationCounter = 0;
        }
        
        PIR1bits.CCP1IF = 0;
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

