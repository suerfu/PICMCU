/* 
 * File:   functions.h
 * Author: suerfu
 *
 * Created on December 27, 2022, 5:15 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// ****************************************************************************
// Some global variables:
// ****************************************************************************

// Time keeping
unsigned int milisecond = 0;
unsigned char second = 0;
unsigned int minute = 0;

unsigned int wait_time = 0;

unsigned int time_to_sleep = 3;     // time in minutes before device enters sleep mode

// Important flags
unsigned char calibration = 0;  // calibration mode
unsigned char display = 0;      // if true, the nixie column should be lit up
unsigned char measure = 0;      // if true, device makes a measurement and resets it

// Parameters
#define SAMPLE_INTERVAL 2

// Dynamic variables for smooth control of the light column
unsigned char cur_temp_pwm = 0;     // current temperature PWM value
unsigned char target_temp_pwm = 0;  // target/desired temperature PWM value
unsigned char cur_hum_pwm = 0;
unsigned char target_hum_pwm = 0;

// ****************************************************************************
// Configuration functions.
// ****************************************************************************

// IO operations
//
void ConfigPort(){
    // Default on startup
    PORTA = PORTC = 0;
    PORTB = 0b01;
    
    TRISA = TRISC = 0xff;
    TRISB = 0xfc;   // 0 for HV power, 1 for sensor power

    ANSELA = ANSELB = ANSELC = 0;
}

// Clock-related
//
void ConfigClock(){
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    while( (OSCSTAT & 0x58) != 0x58 ){;}
}

// Timer 0 is an 8-bit timer that runs on instruction clock + optional pre-scalar
/*
void ConfigTimer0(){
    OPTION_REGbits.T0CS = 0;
        // system's instruction clock
        // 1 ==> external T0CKI pin
    OPTION_REGbits.PSA = 0;
        // enable precalar
    OPTION_REGbits.PS = 0b111;
        // prescalar set to 256
    OPTION_REGbits.PS = 0b101;
        // prescalar set to 64
    OPTION_REGbits.PS = 0b100;
        // prescalar set to 16
}
*/

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
void ConfigCCP1(){
    CCP1CONbits.CCP1M = 0xb;     // compare with software interrupt
    CCPR1H = 1000 / 256;
    CCPR1L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
}

//==================================================================================================
// Serial Interface
// Configured for 9600 8N1 mode.
//
void ConfigUSART(){
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

//==================================================================================================
// Analog ADC-related functions
//==================================================================================================

struct twobytes{
    unsigned char lsb;
    unsigned char msb;
};

union word{
    struct twobytes bytes;
    unsigned int val;
};

void ConfigADC(){
    
    // ports used for analog ADC functions.
    ANSELAbits.ANSA0 = 1;   // A0 used for temperature sensor
    ANSELAbits.ANSA1 = 1;   // A1 humidity sensor
    ANSELAbits.ANSA2 = 1;   // temperature voltage after PWM filtering
    ANSELAbits.ANSA3 = 1;   // positive reference, which will be measured and used as soft reference
    ANSELAbits.ANSA5 = 1;   // humidity voltage after PWM filtering
    
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

float ReadPositiveReference(){
    return ReadADCChannel(3);
}

float ReadTemperature(){    
    float ratio = ReadADCChannel(0);
    ratio /= ReadPositiveReference();
    return (ratio*5-1.375)/0.0225;
}

float ReadHumidity(){    
    float ratio = ReadADCChannel(1);
    ratio /= ReadPositiveReference();
    return (ratio-0.1515)/0.00636;
}

float ReadTemperaturePWM(){    
    return ReadADCChannel(2)/ReadPositiveReference();
}

float ReadHumidityPWM(){    
    return ReadADCChannel(5)/ReadPositiveReference();
}


//==================================================================================================
// PSMC and output control
//==================================================================================================

void ConfigPSMC1(){
    
    // Period, duty cycle and phase
    // Most values will be default and won't be changed in the program.
    
    // Period
    PSMC1PRH = 0;
    PSMC1PRL = 0xff;
        // 8-bit resolution
    
    // Duty cycle
    PSMC1DCH = 0;
    PSMC1DCL = 0;
    
    // Phase/start of active pulse
    PSMC1PHH = 0;
    PSMC1PHL = 0;
    
    // Set PSMC clock
    PSMC1CLK = 0x00;
    
    // Output port, polarity and enable output
    PSMC1STR0 = 0xff;        
    PSMC1POLbits.P1POLB = 0;    // 0 for active high
    PSMC1OENbits.P1OEB = 1;    // output enable bit
    
    // Set source of event for period, phase and duty cycle counter
    PSMC1PRS = 0x1;
    PSMC1PHS = 0x1;
    PSMC1DCS = 0x1;
    
    // enable steering and load the values, and enable digital driver
    PSMC1CON = 0b11000000;  
    
    // disable the tri-state
    TRISCbits.TRISC1 = 0;   
}

void ConfigPSMC3(){
    // Period
    PSMC3PRH = 0;
    PSMC3PRL = 0xff;    
    // Duty cycle
    PSMC3DCH = 0;
    PSMC3DCL = 0;
    // Phase/start of active pulse
    PSMC3PHH = 0;
    PSMC3PHL = 0;
    // Set PSMC clock
    PSMC3CLK = 0x00;
    // Output port, polarity and enable output
    PSMC3STR0 = 0xff;
    PSMC3POLbits.P3POLB = 0;    // 0 for active high
    PSMC3OENbits.P3OEB = 1;    // output enable bit
    // Set source of event for period, phase and duty cycle counter
    PSMC3PRS = 0x1;
    PSMC3PHS = 0x1;
    PSMC3DCS = 0x1;
    // enable steering and load the values, and enable digital driver
    PSMC3CON = 0b11000000;
    TRISCbits.TRISC2 = 0;
}

void SetTemperaturePWM( char DCL ){
    PSMC1DCL = DCL;
    PSMC1CON = 0b11000000;
        // Load the values
}

void SetHumidityPWM( char DCL ){
    PSMC3DCL = DCL;
    PSMC3CON = 0b11000000;
}

// Function to calculate desired PWM corresponding to the float temperature
unsigned char CalculateTemperaturePWM( float temp ){
    if( temp<-3 ){
        return 0;
    }
    else if(temp>40){
        return 219;
    }
    else{
        return 16.6+5.04*temp;
        // coefficients determined by calibration
    }
}

// Function to calculate desired PWM corresponding to the float humifity
unsigned char CalculateHumidityPWM( float hum ){
    if( hum<0 ){
        return 0;
    }
    else if(hum>100){
        return 0;
    }
    else{
        return 19.8+2.08*hum;
        // coefficients determined by calibration
    }
}

// A short function to increment or decrement current value towards target value
// This is to ensure the Nixie column lights up from the bottom.
unsigned char AdjustPWM( unsigned char target, unsigned char cur){
    if( cur<target ){
        return cur+1;
    }
    else if( cur>target ){
        return cur-1;
    }
    else return cur;
}


//==================================================================================================
// Interrupt-related functions
//==================================================================================================

void ConfigInterrupt(){   

    INTCON = 0x0;
    
    // ------------- CCP module -------------
    //
    INTCONbits.PEIE = 0x1;
        // enable peripheral
    PIE1 = 0x0;
    PIE1bits.CCP1IE = 0x1;
        // enable CCP1
    
    // ------------- IOC module -------------
    // interrupt on change
    //
    INTCONbits.IOCIE = 1;
    IOCAP = IOCAN = IOCBP = IOCBN = IOCCP = IOCCN = 0;
    IOCCPbits.IOCCP0 = 1;   // user interrupt via button
        
    // enable global interrupt
    INTCONbits.GIE = 1;     // this should be done in the very end
}

void Initialize();

void __interrupt() handler(){  

    if( PIR1bits.CCP1IF==1 ){

        // increment software timer
        milisecond++;
        if( milisecond >= 1000 ){
            milisecond = 0;
            second++;
            if( second >= 60 ){
                second = 0;
                minute++;
            }
        }
            
        if( (display>0) && (milisecond%20 == 0) ){
            cur_temp_pwm = AdjustPWM( target_temp_pwm, cur_temp_pwm );
            cur_hum_pwm = AdjustPWM( target_hum_pwm, cur_hum_pwm );
            SetTemperaturePWM( cur_temp_pwm );
            SetHumidityPWM( cur_hum_pwm );
        }
            
        if( (display>0) && (milisecond==0) && (second==SAMPLE_INTERVAL) ){
            measure = 1;
        }
        
        if( wait_time>0 ){
            wait_time -= 1;
        }
        
        PIR1bits.CCP1IF = 0;
    }
    
    if( INTCONbits.IOCIF==1 && IOCCFbits.IOCCF0==1 ){
        Initialize();
        IOCCFbits.IOCCF0 = 0;
    }
    
    return;
}

// Turns the power of sensors on and off
//
void SensorPower( char a ){
    // if >0, power up peripheral sensors directly
    if( a>0 ){
        PORTBbits.RB1 = 1;
    }
    // power off peripherals to save power and service time
    else{
        PORTBbits.RB1 = 0;
    }
}

// Returns the status of the sensor power
char GetSensorPower(){
    return PORTBbits.RB1;
}

// Turns the high voltage for Nixie tube on and off
void NixiePower( char a ){
    if( a>0 ){
        PORTBbits.RB0 = 0;
    }
    else{
        PORTBbits.RB0 = 1;
    }
}

// Initialize global variables used in this program
//
void Initialize(){

    // time-related
    milisecond = 0;
    second = 0;
    minute = 0;
    
    // display-related
    display = 1;    // if true, Nixie tube will be driven
    measure = 1;    // if true, Nixie tube will be updated with measurement values
    calibration = 0;    // by default, not in calibration mode
    
    cur_temp_pwm = 0;
    cur_hum_pwm = 0;
    
    SensorPower(1);
    NixiePower(1);
}
#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

