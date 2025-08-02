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
    SSP1STAT = 0x80;    // 0x00 also works (2024-10-15)
    SSP1STAT = 0x00;
    
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x0;
    SSP1CON3 = 0x0;
    
    // Configure baud rate
    //
    //SSP1ADD = 0x4f; // for 100 kHz operation
    SSP1ADD = 0x13; // 400 kHz operation
        // 400 kHz also works (2024-10-15)
}


void I2C_Idle(){
    while( (SSP1STAT & 0x05) || (SSP1CON2 & 0x1F) );
        // SSP1STAT bit 4 is R_nW, used to indicate transmission in progress
        // this OR-ing is based on PIC MCU documentation SSPSTAT register description
}


void I2C_Start(){
    I2C_Idle();
    SSPCON2bits.SEN = 1;
    while( SSPCON2bits.SEN ){;}
    PIR1bits.SSP1IF = 0;
        // SEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
        // Both the two above lines are essential (2024-10-15)
}


void I2C_ReStart(){
    I2C_Idle();
    SSPCON2bits.RSEN = 1;
    while( SSPCON2bits.RSEN ){;}
    PIR1bits.SSP1IF = 0;
        // SEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
        // Both the two above lines are essential (2024-10-15)
}

void I2C_Stop(){
    I2C_Idle();
    SSPCON2bits.PEN = 1;
    while( SSPCON2bits.PEN ){;}
    PIR1bits.SSP1IF = 0;
        // PEN is cleared by hardware automatically
        // SSP1IF needs to be cleared manually
    char a = SSPBUF;
}


// Write a single byte to I2C
//
char I2C_WriteByte( char d ){
    I2C_Idle();
    SSPBUF = d;
    I2C_Idle();
    //while ( !PIR1bits.SSP1IF ){;}
    //PIR1bits.SSP1IF = 0;
        // wait for interrupt flag
        // this wait for finish flag is also essential for proper function (2024-10-15)
    return SSPCON2bits.ACKSTAT;
}


// Read a single byte from I2C
// Assumes the I2C bus is already in the middle of a transaction and slave is sending data
// Input ack => whether acknowledge receipt of data
//
char I2C_ReadByte( char ack ){
    I2C_Idle();
    SSPCON2bits.RCEN = 1;
    
    //while ( !PIR1bits.SSP1IF ){;}
        // wait for interrupt flag
        // this wait for finish flag is also essential for proper function (2024-10-15)
    I2C_Idle();
    
    char tmp = SSPBUF;
    
    SSP1CON2bits.ACKDT = ack?0:1;
    SSP1CON2bits.ACKEN = 1;
    
    return SSPBUF;
}


// Write an entire sequence of data to address
//
char I2C_Write( char addr, char* data, char len){
    
    I2C_Start();
    
    char a = I2C_WriteByte( addr & 0xfe );
    if( a!= 0){
        printf("Error: device not found at address %x\n\r", addr);
    }
    else{
        for( char j=0; j<len; j++){
            char a = I2C_WriteByte( data[j] );
            if( a!=0 ){
                printf("I2C Write: %x not acknowledged by %x\n\r", data[j], addr );
            }
        }
    }
    
    I2C_Stop();
}


char I2C_Read( char addr, char* wdata, char wlen, char* rdata, char rlen){
    
    I2C_Start();
    
    char a = I2C_WriteByte( addr & 0xfe );
    if( a!= 0){
        printf("Error: device not found at address %x\n\r", addr);
    }
    else{
        for( char j=0; j<wlen; j++){
            char a = I2C_WriteByte( wdata[j] );
            if( a!=0 ){
                printf("I2C Write: %x not acknowledged by %x\n\r", wdata[j], addr );
            }
        }
    
        I2C_ReStart();
        I2C_WriteByte( addr | 0x1 );
        for( char j=0; j<rlen; j++){
            rdata[j] = I2C_ReadByte( j==rlen-1?0:1 );
        }
    }
    
    I2C_Stop();
}

#ifdef	__cplusplus
}
#endif

#endif	/* MSSP_H */

