/* 
 * File:   functions.h
 * Author: suerfu
 * Created on December 8, 2021, 7:18 PM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

// PORTx is the actual value.
// LATx is the intended value, which could be different from port value, depending on tri-state (TRIS)
// TRISx pin high will make the pin an input pin, and low => output.
// ANSELx is associated with pins with analog functions. Setting it to high will block digital and connect the pin to analog circuitry.
// 
void ConfigPort(){
    TRISA = TRISB = TRISC = 0;
    ANSELA = ANSELB = ANSELC = 0;
    PORTA = PORTB = PORTC = 0x0;
}


// Oscillator module / Clock
// Eight modes: external lo/med/high, crystal lo/med/high, ext RC, internal
// For internal clock sources:
//    SCS (system clock select) in OSCCON controlls clock source, and internal oscillator frequency select bits IRCF<3:0>
//    SCS<1:0>: 1x = internal oscillator block
//              01 = Timer1 oscillator
//              00 = system clock of Fosc<2:0> in configuration word
//              Note: when using other frequencies in HF, MF and LF, it should be in 1x.
//                    only 32 MHz should use 00
//    IRCF<3:0>: 1111 = 16 / 32 MHz, depending on PLL
//               1110 = 8 or 32 MHz
//    To enable 4xPLL, set PLLEN in config word to be true, and set SPLLEN bit in OSCCON register to be true.  
//
//    Note: which clock to use is determined by the FOSC bits.
//    
void ConfigClock(){
    
    OSCCONbits.SCS = 0x0;
    OSCCONbits.SPLLEN = 1;
    OSCCONbits.IRCF = 0xf; // 8 MHz internal clock, later PLL to 32 MHz
    
    while( (OSCSTAT & 0x90) != 0x90 ){;}
        // Test if stable
        // OSCSTAT is: T1OSC, PLL, OST-START-UP, HF-ready, HF-locked, MF-ready, LF-ready, HF-stable
}


// UART
// Baud rate control (BAUDCON): SYNC, BRGH, BRG16, SPBRGH-SPGRGL controls the baud rate.
    
// Transmit status and control (TXSTA)
//      TXEN = 1, SYNC = 0 (asynchronous), SPEN (of RCSTA) = 1 (enables EUSART and configures TX pin)
//      * analog function must be disabled.
//    Data transmission by writing TXREG register.
//    Polarity: SCKP bit of BAUDCON: 0 => high true idle
//    Interrupt: TXIF bit of PIR1 register, set when no character is held for transmission in TXREG => it is clear when busy. It is set regardless of interrupt.
//    9-bit:
//      Set TX9 bit of the TXSTA register to enable. TX9D bit of TXSTA is the actual data.
    
// Receive status and control (RCSTA)
//      CREN = 1 (enable receiver), SYNC = 0, SPEN = 1, user must manually set TRIS bit to configure RX pin.
//    Receive the data by reading RCREG register
//    Interrupt: RCIF of PIR1 is set when there is unread character
//    9-bit: EX9 bit in RCSTA to enable, data in RX9D bit of RCSTA
//    
void ConfigUSART(){
    
    TRISCbits.TRISC7 = 1;  // pin 7 for input
    TRISCbits.TRISC6 = 0;   // pin 6 for output
    ANSELCbits.ANSC7 = 0;
    ANSELCbits.ANSC6 = 0;
    
    TXSTA = 0x24; // 2 for TXEN, 4 for BRGH (high baud rate)
    RCSTA |= 0x90; // 9 for SPEN and CREN
    
    // Set baud rate for 9600 by default
    BAUDCON = 0x08; // use 16-bit baud rate generator
    SPBRGH = 832/256;
    SPBRGL = 832%256; // for these numbers, refer to table 28-5 in data sheet
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


// Fixed voltage reference: 1.024 V, 2.048 V, 4.096 V, can route to ADC pin, ADC ref, DAC and comparator
//    enable by setting FVREN bit of FVRCON
//    FVRRDY bit of FVRCON to check if stablized
//    ADFVR<1:0> used to set gain when it is routed to ADC pins, 11 is x4, 10 is x2
//    CDAFVR<1:0> used to set gain when it is routed to DAC pins, 11 is x4, 10 is x2
//
void ConfigFVR(){
    FVRCONbits.FVREN = 1;
    FVRCONbits.CDAFVR = 0b11;
        // x4 amplification
    while( FVRCONbits.FVRRDY!=1 ){;}
}
    
// Analog-to-Digital converter
// 
// Enable by ADON bit of ADCON0.
// Port configuration: TRIS must set to high, and ANSEL selected.
// Channel selection:
//    CHS<4:0> of ADCON0 determines which positive input channel.
//        11111 = FVR buffer
//        11101 = temperature indicator
//        00000 = AN0
//    CHSN<3:0> of ADCON2 determines negative channel, 1111 => single ended.
// Voltage reference:
//    ADPREF of ADCON1 determines positive reference (01 = Vref, 00 = Vdd, 11 = FVR buffer 1).
//    ADNREF of ADCON1 determines negative reference (0 for Vss, 1 for Vref- pin).
// Conversion clock:
//    ADCS of ADCON1, it can be Fosc/2 ... Focs/64, note some values are not recommended.
//    When 32 MHz clock is used, Fosc/32 (010) and Fosc/64 (110) are recommended
// Result formatting:
//    ADFM of ADCON1, sign and magnitude (0, lsb is sign, use 0 to fill right) vs 2's compliment (1, use MSB to fill left)
//    ADRMD of ADCON0 10-bit or 12-bit (0 for 12-bit)
// Start by writing 1 to GO bit of ADCON0. When conversion is finished, the GO bit will be cleared.
// Result accessed in ADRESH and ADRESL
// 
void ConfigADC(){
    
    // port configuration
    TRISA |= 0x0f;
    ANSELA |= 0x0f;
    
    // voltage references
    ADCON1bits.ADPREF = 0b11;       // use FVR as input reference
    ADCON1bits.ADNREF = 0;          // use ground as negative reference
    
    // clock
    ADCON1bits.ADCS = 0b110;            // Clock = Fosc/64
    
    // result formatting
    ADCON0bits.ADRMD = 0;           // 12-bit resolution
    ADCON1bits.ADFM = 0;            // sign and magnitude
    
    // enable ADC
    ADCON0bits.ADON = 1;
}

    
void ConfigADCChannel( char chan ){
    if( chan>3 ){
        return;
    }
    else{
        // If current channel is not the specified channel
        // set the channel (A0--A3) and throw away 1 conversion to stabilize (meet acquisition time requirement)
        if( ADCON0bits.CHS != (chan & 0x1f) ){
            ADCON0bits.CHS = (chan & 0x1f);
            ReadADC();
        }
        // Set the negative input as part of differential inputs.
        ADCON2bits.CHSN = 0b1111;   // minus input is negative reference, effectively single ended.

    }
}

    
unsigned int ReadADC(){
    ADCON0bits.GO = 1;
        // start ADC
    while( ADCON0bits.GO == 1 ){;}
        // wait for conversion to be done
    int result = ADRESH;
    result = (result << 4);
    result += (ADRESL >> 4);
        // Currently not possible to have negative values
    return result;
}


// Digital-to-analog converter 8-bit (DAC1, connected to AN2 pin)
//
// Enable with DAC1EN bit of DAC1CON0 register
// Voltage set with DAC1R<7:0> bits of DAC1CON1 register
//      Vout = [ (Vsrc+ - Vsrc-) * DAC1R/2^8 ] + Vsrc-
// Output enabled by DAC1OE1 bit of DAC1CON0 register
// DAC1PSS<1:0> selects Vsrc+
//    10 = FVR buffer 2, 01 = Vref+ ext pin, 00 = Vdd
// DAC1NSS selects Vsrc-
//    1 = Vref- pin, 0 = Vss


// Pulse-width Modulation
//
// Procedure:
//      disable the output
//      set period through timer2
//      set the mode and pulse width
//      enable digital drive.
//  
// Note: seems all the steps need to be followed to change PWM waveform
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
    CCP1CONbits.CCP1M = 0xff;
        // 11xx is PWM mode
        // other configs are compare, compare-auto-ADC, capture on every n-th edge.
    
    // 
    CCP1CONbits.DC1B = low;
        // least significant two bits for PWM pulse width
    CCPR1L = high;
        // most significant 8 bits of the 10-bit words.
    
    // Enable the drive
    TRISCbits.TRISC2 = 0;
}
#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */
