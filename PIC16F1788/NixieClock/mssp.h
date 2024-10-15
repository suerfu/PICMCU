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

// Configure port and bits for I2C
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
    SSP1STAT = 0x80;
    //SSPCON1bits.SSPM = 0x8;
    //SSPCON1bits.SSPEN = 1;
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x0;
    SSP1CON3 = 0x0;
    
    // Configure baud rate
    //
    SSP1ADD = 0x4f; // for 100 kHz operation
    //SSP1ADD = 0x13; // 400 kHz operation
}


void I2C_Master_Wait(){
    while( (SSP1STAT & 0x01) || (SSP1CON2 & 0x1F) );
}


void I2C_Master_Start(){
    I2C_Master_Wait();
    SSPCON2bits.SEN = 1;
    while( SSPCON2bits.SEN ){;}
    PIR1bits.SSP1IF = 0;
        // SEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
    /*
    if( PIR1bits.SSP1IF==1 ){
        printf("Flag is on\n\r");
        PIR1bits.SSP1IF = 0;
    }
     */
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
    //while( SSP1STATbits.BF );
    I2C_Master_Wait();
    SSPBUF = d;
    // wait for interrupt flag
    while ( !PIR1bits.SSP1IF ){;}
    return SSPCON2bits.ACKSTAT;
}



/*
int I2CWaitAck(){

    for( char i=0; i<255; i++){
        if( SSPCON2bits.ACKSTAT==0 ){
            return 0;
        }
    }
    return -1;  // slave not present/data not received
}
*/

int I2CWrite( char addr, char* data, char n){
    
    I2C_Master_Start();
    
    //while( SSPCON2bits.SEN==1 );
    //I2CIFPoll();

    // write address and write-bit
    I2C_Master_Wait();
    SSP1BUF = ( addr & 0xfe );
    //while( SSPSTATbits.BF == 1 ){}

    // check ack status
    //int ack = I2CWaitAck();
    //if( ack<0 ){
    //    return -1;  // slave not present
    //}
    
    // write data for transmission
    for( int i=0; i<n; i++){
        I2C_Master_Wait();
        SSP1BUF = data[i];
        /*while( SSPSTATbits.BF == 1 ){}
        ack = I2CWaitAck();
        if( ack<0 ){
            return -2;  // slave present but data not received
        }*/
    }

    return 0;
}



#ifdef	__cplusplus
}
#endif

#endif	/* MSSP_H */

