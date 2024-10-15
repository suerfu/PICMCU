/* 
 * File:   mssp.h
 * Author: suerfu
 *
 * Created on October 15, 2024, 10:42 AM
 */

#ifndef MSSP_H
#define	MSSP_H

#ifdef	__cplusplus
extern "C" {
#endif



// I2C
//
//  SSPCON1: WCOL | SSPOV | SSPEN | CKP | SSPM<3:0>
//      WCOL: write collision, attempt to write SSPBUF while I2C line is not ready, need to be cleared manually
//      SSPOV: receive overflow, byte is received while previous byte is still in the register
//      SSPEN: synchronous serial port enable, 1 to enable either SPI or I2C
//      CKP: clock parity selection, not used in I2C master mode
//      SSPM<3:0>:
//          1111 = I2C slave, 10-bit address, w/ start/stop bit interrupt
//          1110 = I2C slave, 7-bit address, w/ start/stop bit interrupt
//          1011 = I2C firmware controlled master
//          1000 = I2C master mode, clock = Fosc / (4 * (SSPADD+1) )
//          0111 = I2C slave, 10-bit
//          0110 = I2C slave, 7-bit
//      *SPI-related modes are omitted
//
//  SSPCON2: GCEN | ACKSTAT | ACKDT | ACKEN | RCEN | PEN | RSEN | SEN
//      GCEN: general call enable, only used in I2C slave
//      ACKSTAT: acknowledge status, read-only, 
//          1 = ack not received, 0 = ack received
//      ACKDT: acknowledge data in receive mode, value transmitted when acknowledge sequence initiated
//          0 = received, 1 = not received
//      ACKEN: enable acknowledge sequence (I2C master mode only)
//          1 = initiate acknowledge sequence and transmit the value in ACKDT
//      RCEN: 1 = enable receive mode in I2C master
//      PEN: 1 = initiate stop condition in I2C master
//      RSEN: 1 = enable repeated start condition in I2C master
//      SEN: 1 = initiate start condition (I2C master)
//
//  SSPCON3: mostly used only in I2C mode except one bit to control data hold time
//
//  SSPADD: used to configure baud rate, which is set as freq = (Fosc/4) / (SSPADD+1)
//      SSPADD = 0x4F to configure 100 kHz rate when Fosc = 32 MHz
//
void ConfigI2C(){
    
    // Configure port for IO
    //
    TRISCbits.TRISC3 = 1;   // input
    TRISCbits.TRISC4 = 1;
    ANSELCbits.ANSC3 = 0;   // disable analog functions
    ANSELCbits.ANSC4 = 0;

    // Configure mode for I2C
    //
    SSP1STAT = 0x80;    // 0x00 also works (2024-10-15)
    
    //SSPCON1bits.SSPM = 0x8;
    //SSPCON1bits.SSPEN = 1;
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x0;
    SSP1CON3 = 0x0;
    
    // Configure baud rate
    //
    //SSP1ADD = 0x4f; // for 100 kHz operation
    SSP1ADD = 0x13; // 400 kHz operation
        // 400 kHz also works (2024-10-15)
}


void I2C_Master_Wait(){
    while( (SSP1STAT & 0x01) || (SSP1CON2 & 0x1F) );
        // it seems this wait function is not very essential to the function
}


void I2C_Master_Start(){
    I2C_Master_Wait();
    SSPCON2bits.SEN = 1;
    while( SSPCON2bits.SEN ){;}
    PIR1bits.SSP1IF = 0;
        // SEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
        // Both the two above lines are essential (2024-10-15)
}


void I2C_Master_Stop(){
    I2C_Master_Wait();
    SSPCON2bits.PEN = 1;
    while( SSPCON2bits.PEN ){;}
    PIR1bits.SSP1IF = 0;
        // PEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
}


char I2C_Master_Write( char d ){
    I2C_Master_Wait();
    SSPBUF = d;
    while ( !PIR1bits.SSP1IF ){;}
        // wait for interrupt flag
        // this wait for finish flag is also essential for proper function (2024-10-15)
    return SSPCON2bits.ACKSTAT;
}



#ifdef	__cplusplus
}
#endif

#endif	/* MSSP_H */

