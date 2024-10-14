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


// Timer1
// 16-bit counter
// To enable:
//  * disable TMR1GE bit of T1GCON to disable counter/gated mode
//  * select clock source via TMR1CS<1:0> and T1OSCEN bit of T1CON
//      * 11 and x => LFINTOSC
//      * 10 and 0 => external clock
//      * 01 and x => system clock (Fosc)
//      * 00 and x => instruction clock (Fosc/4)
//  * select prescalar via T1CKPS of T1CON
//      * 1,2,4 or 8, increasing order (00,01,10,11)
//  * enable TMR1ON bit of T1CON
//
void ConfigTimer1(){
    T1CONbits.TMR1CS = 0x0;
        // clock source is instruction clock Fosc/4
    T1CONbits.T1CKPS = 0x3;
        //Fosc = 32 MHz. Timer clock is Focs/4, another pre-scaler by 8 to obtain 1 MHz timer.
    T1GCONbits.TMR1GE = 0;
    T1CONbits.TMR1ON = 1;
}


// Capture and compare module
// Capture mode: allows timing of events triggered via external pins
// Compare mode: allows user to trigger external event when predetermined time has passed
// Event is defined by CCPxM<3:0> bits of CCPxCON register
//  * x can be 1, 2, or 3, which corresponds to three different modules.
//      * 11xx: PWM mode
//      * 1011: compare mode, ADC auto-conversion trigger
//      * 1010: compare mode, software interrupt only （Note: there might be bug here, the timing not working as expected）
//      * 1001: compare mode, clears output pin (a physical pin in port A, B or C)
//      * 1000: compare mode, sets output pin (a physical pin in port A, B or C)
//      * 0111: capture mode, every 16th rising edge
//      * 1000: capture mode, every 4th rising edge
//      * 1000: capture mode, every rising edge
//      * 1000: capture mode, every falling edge
//      * 0010: compare mode, toggle output
//      * 0000: module disabled
//
void ConfigCCP1(){
    CCP1CONbits.CCP1M = 0xb;     // compare with software interrupt
    CCPR1H = 1000 / 256;
    CCPR1L = 1000 % 256;
        // calling the interrupt handler every 10000 clock ticks, that is 20Hz frequency.
}



// Fixed voltage reference: 1.024 V, 2.048 V, 4.096 V, can route to ADC pin, ADC ref, DAC and comparator
//    enable by setting FVREN bit of FVRCON
//    FVRRDY bit of FVRCON to check if stablized
//    ADFVR<1:0> used to set gain when it is routed to ADC pins, 11 is x4, 10 is x2
//    CDAFVR<1:0> used to set gain when it is routed to DAC pins, 11 is x4, 10 is x2
//
void ConfigFVR(){
    FVRCONbits.FVREN = 1;
    FVRCONbits.ADFVR = 0b11;
        // x4 amplification for ADC
    FVRCONbits.CDAFVR = 0b11;
        // x4 amplification for DAC/compare
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
    
    // Positive and negative references
    // Should be changed based on applications.
    ADCON0bits.CHS = 1;
    ADCON2bits.CHSN = 0b1111;
    
    // clock
    ADCON1bits.ADCS = 0b010;        // Clock = Fosc/32, fastest recommended clock
    
    // result formatting
    ADCON0bits.ADRMD = 0;           // 12-bit resolution
    ADCON1bits.ADFM = 0;            // sign and magnitude
    
    // enable ADC
    ADCON0bits.ADON = 1;
}

    
/*
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
*/

    
// ADC result is accessed as bytes but full resolution is more than a byte.
// This is a trick to access two bytes as a 16-bit unsigned integer
//
struct twobytes{
    unsigned char lsb;
    unsigned char msb;
};
    
union word{
    struct twobytes bytes;
    unsigned int val;
};


// Read the conversion result
// Note: if channel is changed, enough time should be allowed for voltages to stabilize
//      A convenient way is to do a readout and throw it away.
    
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
    CCP1CONbits.CCP1M = 0xf;
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


// PSMC 
// Higher performance & resolution PWM module.
// Multiple modules (index n)
// Each module can have multiple outputs A--F, which can be a single replica of the waveform or more complex patterns.
//
// Procedure:
//      set period, duty cycle and phase (start of active pulse)
//      set PSMC clock source
//      set output port and polarity, enable output
//          each port A--F can be configured here
//      source for event (what causes start of a new period and begin and end of an active pulse)
//      load the LD bit to push the register values through the buffer and enable digital drive
//    
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
    PSMC1STR0 = 0xff;
        // output the waveform on all the pins A-F
        // this register controls what each of pins A-F are doing (single PWM, complementary, etc.)
    //PSMC1STR1bits.P1SSYNC = 0x0;
        // sync output immediately
        
    PSMC1POLbits.P1POLA = 0;    // 0 for active high
    PSMC1OENbits.P1OEA = 1;    // output enable bit
    
    // Set source of event for period, phase and duty cycle counter
    PSMC1PRS = 0x1;
    PSMC1PHS = 0x1;
    PSMC1DCS = 0x1;
        // LSB = 1 for enabling synchronous event triggered by a match with TMR, disabling all other modes.
    
    // enable steering and load the values, and enable digital driver
    PSMC1CON = 0b11000000;
        // bit7 is enable bit, bit6 is Load, need to clear to enable
        // bit0-3 set to 0 for single PWM output
    TRISCbits.TRISC0 = 0;
        // previously, setting the entire TRISC = 0 was affecting the operation of UART
        // this should be changed based on which output is used.
}


//  Below is a shorter version of the config function
//  This function changes only period and duty cycle
//  It assumes other attributes (port, event source, clk, etc.) have already been configured and need not change.
//
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


// Interrupt
// To enable:
//  * enable interrupt enable bits of the specific interrupt"
//      * INTCON has PEIE (peripheral), TMR0IE, INTE (external), IOCIE (upon change), TMR0IF, INTIF and IOCIF
//          * IOCIF is readonly: should clear individual pin's flag
//      * Under PEIE PIE1 register has:
//          * TMR1GE, ADIE (ADC), RCIE (UART), TXIE (UART), SSP1IE, CCP1IE, TMR2IE, TMR1IE
//          * its corresponding flag register is PIR1 which contains the corresponding flag bits
//              * TMR1GIF, ADIF, RCIF, TXIF, SSP1IF, CCP1IF, TMR2IF, TMR1IF
//  * implement interrupt handler (syntax: void __interrupt() handler(){...})
//      * must check and clear individual interrupt flags to see what triggered the interrupt
//      * if interrupt comes from IOC (interrupt on change), should clear the individual pin's flag; the overall flag is read-only.
//  * enable GIE bit of INTCON (this has to be done at the end of configuration)
//
// Interrupt on change (IOC)
// To configure:
//  * enable IOCIE bit of INTCON
//  * configure IOCxP and IOCxN (x is A B or C port, P is trigger on rising edge, and N for falling edge)
// In the interrupt handler, one should check IOCxIF register (remember not to accidentally mask other flags by clearing the entire register).
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
        
        if( milisecond >= 1000 ){
            milisecond = 0;
            second++;
        }
        
        if( second >=60 ){
            second = 0;
            minutes++;
            printf("%d\n\r", radiationCounter);
            radiationCounter = 0;
        }
        
        PIR1bits.CCP1IF = 0;
    }
    else if( INTCONbits.IOCIF==1 && IOCAFbits.IOCAF3==1 ){
        
        BuzzOnOff(1);
        buzzCounter = buzzDuration;
        radiationCounter++;
        
        IOCAFbits.IOCAF3 = 0;
    }
    
    return;
}

    
// EEPROM 
// PIC16F1788 has 1-byte memories addressed with 1-byte (256 memory locations)
// To read memory,
//  * load address
//  * clear EEPGD (0 - not program memory) and CFGS (0 - not device configuration)
//  * set RD bit of EECON high to initiate read.
//      * upon finish, this bit will become low again
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

// To write memory,
//  * load address and value to write
//  * disable interrupt
//  * set write-enable WREN bit of EECON
//  * go through a particular sequence of writing 55H and then aaH to EECON2
//  * set write-enable low and re-enable global interrupt (if it is enabled)
//
void WriteEEPROM( char addr, char value){
    
    EEADRL = addr;  // memory address
    EEDATL = value; // memory value
    
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
