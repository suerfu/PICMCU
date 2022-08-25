/* 
 * File:   lcd.h
 * Author: suerfu
 *
 * Created on August 24, 2022, 6:25 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

// Pin assignment:
//  A5: EN
//  A6: RS ==> High for data, Low for command
//  A7: RW ==> High for read, Low for write
    
//  B0-7: data
    
//  C2: LCD backlight / PWM

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void LCDCommand( char cmd ){
    PORTAbits.RA5 = 0;    
    PORTB = cmd;
    PORTAbits.RA6 = 0;
    PORTAbits.RA7 = 0;
    PORTAbits.RA5 = 1;
    Delay(1);
    PORTAbits.RA5 = 0;
    Delay(1);
}

void LCDData( char data ){
    PORTAbits.RA5 = 1;
    PORTB = data;
    PORTAbits.RA6 = 1;
    PORTAbits.RA7 = 0;
    PORTAbits.RA5 = 0;
}


void ConfigLCD(){
    
    PORTAbits.RA4 = 1;
    
    // Configure direction of data flow
    TRISAbits.TRISA5 = 0;
    TRISAbits.TRISA6 = 0;
    TRISAbits.TRISA7 = 0;
    TRISB = 0;
    TRISCbits.TRISC2 = 0;
    
    // Set default values
    PORTAbits.RA7 = 0;
    PORTCbits.RC2 = 1;
    
    // Allow for setup time of about 40 ms
    Delay( 50 );
    PORTAbits.RA6 = 0;  // RS
    PORTAbits.RA5 = 0;  // EN
    
    // Function set
    LCDCommand( LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS );
    Delay( 5 );
    LCDCommand( LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS );
    Delay( 2 );
    LCDCommand( LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS );

    // Display control
    LCDCommand( LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON );
    
    LCDCommand( LCD_CLEARDISPLAY);
    Delay(2);
    
    LCDCommand( LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT );
    
    PORTAbits.RA4 = 0;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

