/* 
 * File:   port.h
 * Author: suerfu
 * Created on August 3, 2025, 8:44 AM
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


#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */
