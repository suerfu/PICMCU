/* 
 * File:   io.h
 * Author: suerfu
 *
 * Created on October 15, 2024, 2:44 PM
 */

#ifndef IO_H
#define	IO_H

#ifdef	__cplusplus
extern "C" {
#endif

// Set default status of some ports that won't be separately configured
//
void ConfigPort(){
    
    // Port B, 0-5 Nixie digits, 6 - RTC input, 7 - HV shutdown signal (low to enable)
    TRISB = 0x40;
    PORTBbits.RB7 = 1;
    ANSELB = 0;   // disable analog functions on 
    
    // Port A, 0-2 for analog input, 3 for digital output, 4-7 for digital input
    TRISA = 0xf7;    // only port3 is for digital output
    PORTAbits.RA3 = 0; // PortA-3 = power pin for analog sensors
    
    // Port C will be separately configured
}



#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */

