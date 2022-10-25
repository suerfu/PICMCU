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

unsigned int milisecond = 0;
unsigned int second = 0;
unsigned int minutes = 0;

// IO operations
//
void ConfigPort(){
    // Default on startup
    TRISA = TRISB = TRISC = 0xff;
    ANSELA = ANSELB = ANSELC = 0;
    PORTA = PORTB = PORTC = 0;

    TRISBbits.TRISB6 = TRISBbits.TRISB7 = 1;
        // B6 and B7 used to get button inputs
    PORTCbits.RC2 = 1;
    TRISCbits.TRISC2 = 0;
    
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

// CCP module, needed for timer & interrupt
// CCP module NEEDS timer 1
//
void ConfigCCP2(){
    CCP2CONbits.CCP2M = 0xb;     // compare with software interrupt
    CCPR2H = 1000 / 256;
    CCPR2L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
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


// Analog ADC-related functions

struct twobytes{
    unsigned char lsb;
    unsigned char msb;
};

union word{
    struct twobytes bytes;
    unsigned int val;
};


void SensorPower( char a ){
    // if >0, power up peripheral sensors
    // using P-channel mosfet, so input low corresponds to output high.
    if(a>0){
        PORTCbits.RC2 = 0;
    }
    // power off peripherals to save power and service time
    else{
        PORTCbits.RC2 = 1;
    }
}


void ConfigADC(){
    
    // Config ports used for analog ADC functions.
    ANSELAbits.ANSA0 = 1;   // A0 used for temperature sensor
    ANSELAbits.ANSA1 = 1;   // A1 pressure sensor
    ANSELAbits.ANSA2 = 1;   // A2 humidity sensor
    ANSELAbits.ANSA3 = 1;   // A3 ambient light sensor
    
    // voltage references
    //ADCON1bits.ADPREF = 0b11;   // use FVR as input reference
    ADCON1bits.ADPREF = 0;      // use Vdd as positive reference
    ADCON1bits.ADNREF = 0;      // use ground as negative reference
    
    // Negative reference of the differential input channel
    // Positive reference will be changed during the application
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

unsigned int ReadADCChannel( char ch){
    
    if( ADCON0bits.CHS != ch ){
        ADCON0bits.CHS = ch;
        ReadADC();
    }    
    return ReadADC();
}

float ReadTemperature(){    
    float ratio = ReadADCChannel(0);
    ratio /= 4095;
    return (ratio*5-1.375)/0.0225;
}

float ReadPressure(){    
    float ratio = ReadADCChannel(1);
    ratio /= 4095;
    return (ratio+0.32667)/0.01067;
}

float ReadHumidity(){    
    float ratio = ReadADCChannel(2);
    ratio /= 4095;
    return (ratio-0.1515)/0.00636;
}

float ReadLuminosity(){    
    float ratio = ReadADCChannel(3);
    return ratio/4095;
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
        // enable CCP2
    
    // ------------- IOC module -------------
    // interrupt on change
    //
    //INTCONbits.IOCIE = 0x1;
    //IOCAP = IOCAN = IOCBP = IOCBN = IOCCP = IOCCN = 0;
    //IOCAPbits.IOCAP3 = 0x1;
    
    // enable global interrupt
    INTCONbits.GIE = 0x1;
        // this should be done in the very end
}

void __interrupt() handler(){
    
    if( PIR2bits.CCP2IF==1 ){
        
        milisecond++;
     
        if( milisecond >= 1000 ){
            milisecond = 0;
            second++;
            if( second%2==0 ){
                SensorPower(1);
                printf("%u %f %f %f %f\r\n", ReadADCChannel(0), ReadTemperature(), ReadPressure(), ReadHumidity(), ReadLuminosity() );
                //printf("%d %u %u %u %u\r\n", second, ReadADCChannel(0), ReadADCChannel(1), ReadADCChannel(2), ReadADCChannel(3) );
            }
        }
        
        //PIR1bits.CCP1IF = 0;
        PIR2bits.CCP2IF = 0;
    }
    return;
}


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

