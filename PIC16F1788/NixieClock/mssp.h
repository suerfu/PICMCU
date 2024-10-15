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


void I2C_Master_Wait(){
    while( (SSP1STAT & 0x01) || (SSP1CON2 & 0x1F) );
    if (SSP1CON1bits.WCOL){
        printf("Write collision has ocurred!\n\r");
    }
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
    /*
    for( int i=0; i<40; i++){
        if (PIR1bits.SSP1IF != 0){
            printf("Flag set!\n\r");
            break;
        }
        else{
            printf("%d I2C_Master_Write: flag not set yet.\n\r", i);
        }
    }
    return SSPCON2bits.ACKSTAT;
    */
    while ( !PIR1bits.SSP1IF ){;}
    PIR1bits.SSP1IF = 0;
        // wait for interrupt flag
        // this wait for finish flag is also essential for proper function (2024-10-15)
    return SSPCON2bits.ACKSTAT;
}

char I2C_Master_Read(){
    I2C_Master_Wait();
    SSPCON2bits.RCEN = 1;
    
    if ( !PIR1bits.SSP1IF ){
        return SSPBUF;
    }
    
    while ( !PIR1bits.SSP1IF ){;}
        // wait for interrupt flag
        // this wait for finish flag is also essential for proper function (2024-10-15)
    return SSPBUF;
}



#ifdef	__cplusplus
}
#endif

#endif	/* MSSP_H */

