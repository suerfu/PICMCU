// PIC16F1788 Configuration Bit Settings
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)


#include <xc.h>

#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

/*
Pin description:
- PortA:
	- 0: light sense (analog in)
	- 1: HV feedback sense (analog in)
	- 2: HV switch (digital in)
	- 3: Geiger input (digital in)
	- 4: buzzer output (digital out)
	- 5: LCD en (digital out)
	- 6: LCD RS (digital out)
	- 7: LCD RW (digital out)
- PortB:
	- 0-7: LCD data bus
- PortC
	- 0: SPI select 
	- 1: PSMC HV PWM
	- 2: LCD backlight PWM
	- 3: SPI clock
	- 4: SPI data in
	- 5: SPI data out
	- 6,7: UART TX/RX
 */
extern unsigned int milisecond;
extern unsigned int second;
extern unsigned int minutes;

extern unsigned int radiationCounter;

extern const char buzzDuration;
extern char buzzCounter;

int main(int argc, char** argv) {

    ConfigPort();
    ConfigClock();
    
    ConfigTimer1();
    ConfigCCP1();
    
    ConfigUSART();
    
    ConfigFVR();
    ConfigADC();
    
    char command[8];
        // used to obtain command from user    
    
    // Initial values of period and duty cycle
    // Later the initial value should be read from EEPROM of the processor
    union word period;
    period.val = 600;
    union word duty_cycle;
    duty_cycle.val = 140;
        // first device using 2x27 kOhm + 10 kOhm readout resistor and L=33uH is well-suited for 140/600
    
    union word period;
    period.val = 700;
    union word duty_cycle;
    duty_cycle.val = 500;
        // second board first device using a single 100kOhm readout resistor and L=470uH is well-suited for 500/700
    
    ConfigPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0 );
    
    ConfigInterrupt();
    
    printf("Welcome!\n\r");
    
    while(1){
    
        scanf( "%s", command );
        
        if( strncmp(command, "!pr", 3)==0 ){
            scanf( "%u", &period.val );
            SetPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0);
        }
        else if( strncmp(command, "!dc", 3)==0 ){
            scanf( "%u", &duty_cycle.val );
            SetPSMC1( period.bytes.msb, period.bytes.lsb, duty_cycle.bytes.msb, duty_cycle.bytes.lsb, 0, 0);
        }
        else if( strncmp(command, "?pr", 3)==0 ){
            printf("%u\n\r", period.val );
        }
        else if( strncmp(command, "?dc", 3)==0 ){
            printf("%u\n\r", duty_cycle.val );
        }
        else if( strncmp(command, "?hv", 3)==0 ){
            printf("%d\n\r", ReadHV() );
        }
        
    }

    return 0;
}
