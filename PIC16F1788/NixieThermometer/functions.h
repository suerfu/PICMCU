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

unsigned int milisecond = 0;
unsigned int second = 0;
unsigned int minutes = 0;

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
//
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

//==================================================================================================

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
    // if >0, power up peripheral sensors directly
    if( a>0 ){
        PORTBbits.RB1 = 1;
    }
    // power off peripherals to save power and service time
    else{
        PORTBbits.RB1 = 0;
    }
}

void NixiePower( char a ){
    if( a>0 ){
        PORTBbits.RB0 = 0;
    }
    else{
        PORTBbits.RB0 = 1;
    }
}

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
    PIE1 = 0x0;
    PIE1bits.CCP1IE = 0x1;
        // enable CCP2
    
    // ------------- IOC module -------------
    // interrupt on change
    //
    INTCONbits.IOCIE = 1;
    IOCAP = IOCAN = IOCBP = IOCBN = IOCCP = IOCCN = 0;
    
    //IOCAPbits.IOCAP4 = 1;
    //IOCANbits.IOCAN4 = 1;
        // VOC reading
        // this will be enabled in the read function
    
    IOCBPbits.IOCBP6 = 1;
    IOCBPbits.IOCBP7 = 1;
        // user interface
    
    // enable global interrupt
    INTCONbits.GIE = 1;
        // this should be done in the very end
}


char print_output = 0;

void __interrupt() handler(){  
 /*   
    // first increment timer 0.
    // TMR0 is not always on
    //
    if( INTCONbits.TMR0IF==1 ){
        tmr0_counter.bytes.msb ++;
        INTCONbits.TMR0IF = 0;
        //putch('a');
            // clear the interrupt flag
    }
*/
    if( PIR1bits.CCP1IF==1 ){

        milisecond++;
     
        if( milisecond >= 1000 ){
            milisecond = 0;
            second++;
            if( second%2==1 ){
                print_output = 1;
            }
        }        
        PIR1bits.CCP1IF = 0;
    }
    /*
    if( INTCONbits.IOCIF == 1 ){

        if ( IOCAFbits.IOCAF4 == 1 ){
            
            IOCAFbits.IOCAF4 = 0;

            // Rising edge from VOC, could be CO2 or VOC
            // it can also be 1st event in sequence or second in sequence
            if (IOCAPbits.IOCAP4 == 1 ){

                // No matter what, if a rising pulse arrives, start/reset the timer.
                TMR0 = 0;
                tmr0_counter.val = 0;
                
                // if this is the first pulse after VOC sensor is turned on, interrupt needs to be configured to increment on TMR0 overflow 
                if( co2_timer.val==0 && voc_timer.val==0 ){
                    INTCONbits.TMR0IE = 1;
                }

                // configure to detect the falling pulse
                IOCAPbits.IOCAP4 = 0;
                IOCANbits.IOCAN4 = 1;
            }
            
            // Falling edge from VOC
            // could be end of first event or end of entire acquisition
            else if (IOCANbits.IOCAN4 == 1 ){
                
                tmr0_counter.bytes.lsb = TMR0;
                INTCONbits.TMR0IE = 0;
                    // capture the end of the pulse and momentarily disable the timer
                
                // Assign measurement values
                // Period 33.3 ms
                // Clock 8MHz/256 or 8MHz / PS
                // CO2 is 55% to 95%
                // VOC is 5% to 45%
                if( tmr0_counter.val<4166 ){
                    voc_timer.val = tmr0_counter.val;
                    //printf("voc %u\n\r", voc_timer.val);
                }
                else if( tmr0_counter.val>4166 ){
                    co2_timer.val = tmr0_counter.val;
                    //printf("co2 %u\n\r", co2_timer.val);
                }
                
                IOCANbits.IOCAN4 = 0;
                
                // only one value is read, reconfigure for positive edge trigger
                if( co2_timer.val==0 || voc_timer.val==0 ){
                    IOCAPbits.IOCAP4 = 1;
                }
                // if both values have been acquired, disable IOC
                else{
                    IOCAPbits.IOCAP4 = 0;
                        // both data have been taken
                }
            }
        }
        if ( IOCBFbits.IOCBF6 == 1 ){
            IOCBFbits.IOCBF6 = 0;
            printf("6\n\r");
        }
        if ( IOCBFbits.IOCBF7 == 1 ){
            IOCBFbits.IOCBF7 = 0;
            printf("7\n\r");
        }
    }
    */
    return;
}


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

