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
    
#define LCD_RETURNHOME   0x02
    
// Display entry mode
#define LCD_ENTRYMODESET 0x04
// Flags for entry mode
#define LCD_ENTRYRIGHT 0x02
    // Negate for decrementing address (type to left))
#define LCD_ENTRYSHIFTDISPLAY 0x01
    // Negate for not shifting display as one types

// Display control
#define LCD_DISPLAYCONTROL 0x08
// Flags for display control
#define LCD_DISPLAYON 0x04
    // Negate for LCD off
#define LCD_CURSORON 0x02
    // Negate for cursor off
#define LCD_BLINKON 0x01
    // Negate for blinking off

// Cursor control
#define LCD_CURSORSHIFT 0x10
// Flags for display/cursor shift
#define LCD_DISPLAYSHIFT 0x08
    // Negate for cursor shift
#define LCD_MOVERIGHT 0x04
    // Negate for move to left

// Function set
#define LCD_FUNCTIONSET 0x20
// Flags for function set
#define LCD_8BITMODE 0x10
    // Negate for 4-bit mode
#define LCD_2LINE 0x08
    // Negate for 1-line mode
#define LCD_5x10DOTS 0x04
    // Negate for 5x8 dots

// CGRAM and DDRAM Address
// CG - character generator
// DD - display data
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
    
// Device-specific definitions    
#define ROW_INCR 0x40
#define MAX_ROW 2
//#define MAX_COL 0x28
#define MAX_COL 16

extern void Delay( unsigned int );
    
void LCDWrite( char cmd, char mode ){
    PORTAbits.RA6 = mode;
        // select command or data
    PORTAbits.RA7 = 0;
        // read/~write pin, should always be 0 in this application
    PORTB = cmd;
        // PORTB is the actual data to be sent
    PORTAbits.RA5 = 0;
    PORTAbits.RA5 = 1;
    PORTAbits.RA5 = 0;
        // Pulse the enable pin
    Delay(1);
}

void LCDCommand( char cmd ){
    LCDWrite(cmd, 0);
}

void LCDData( char data ){
    LCDWrite( data, 1);
}

void LCDClear(){
    LCDCommand( LCD_CLEARDISPLAY );
    Delay(2);
}

void LCDHome(){
    LCDCommand( LCD_RETURNHOME );
    Delay(2);
}

void LCDCursor( char row, char col){
    LCDCommand( LCD_SETDDRAMADDR | ( (row%MAX_ROW)*ROW_INCR + col%MAX_COL) );
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
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );
    Delay( 5 );
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );
    Delay( 2 );
    LCDCommand( (LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE) & ~LCD_5x10DOTS );

    LCDClear();
    
    // Display control
    LCDCommand( (LCD_DISPLAYCONTROL | LCD_DISPLAYON) & ~LCD_CURSORON & ~LCD_BLINKON );
    
    // Entry mode
    LCDCommand( (LCD_ENTRYMODESET | LCD_ENTRYRIGHT) & ~LCD_ENTRYSHIFTDISPLAY );
    
    // Cursor shift
    // LCDCommand( (LCD_CURSORSHIFT | LCD_MOVERIGHT) & ~LCD_DISPLAYSHIFT );
    
    PORTAbits.RA4 = 0;
}

void LCDPrint( char temp[], char n, char row, char col ){
    LCDCursor( row, col);
    for( char i=0; i<n; i++){
        LCDData( temp[i] );
        Delay(10);
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

