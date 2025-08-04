/* 
 * File:   functions.h
 * Author: suerfu
 * Created on August 3, 2025, 8:47 AM
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

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


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */
