/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>        /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>       /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>   /* C18 General Include File */
#endif

//NEW

#include <math.h> 
#include <time.h>

#if defined(__XC) || defined(HI_TECH_C)

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <string.h> 


#endif

#include "can.h"
#include "uart.h"
#include "config.h"
#include "i2c.h"
#include "adc.h"

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/
static float temperature;  // temp in Celsius

/* i.e. uint8_t <variable_name>; */
// Adjust to system clock
#define _XTAL_FREQ 16000000
//unsigned int oss = 0;

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

void main(void)
{
    /* Initialization functions called below */
    ConfigureOscillator();
    setAnalogIn();
    selectAN0();
    ecan_init();
    uart_init(2400);
    
    /* Enable Interrupts */
    RCONbits.IPEN   = 0;    // Enable interrupt priority
    INTCONbits.GIE  = 1;    // Enable interrupts
    INTCONbits.PEIE = 1;    // Enable peripheral interrupts.
    
    //PIE1bits.RCIE   = 1;    // Enable USART receive interrupt
    PIE3bits.RXB0IE = 1;    // Enable CAN receive buffer 0 interrupt
    //PIE3bits.RXB1IE = 1;    // Enable CAN receive buffer 1 interrupt
    
    TRISDbits.TRISD0 = 0;
    LATDbits.LATD0 = 1;
    
    //Temperature variables
    unsigned int reading;
    float voltage;
    
    /* While doing recurring functions */
    while(1) {
        reading = readADC();
        
        // converting that reading to voltage
        voltage = reading * 5.0;
        voltage /= 1024.0; 
        
        // convert to temperature
        temperature = (voltage - 0.5) * 100 ;  // converting from 10 mv per degree with 500 mV offset
                                                    // to degrees ((voltage - 500mV) times 100)
        if (temperature > 24.0) {
            uart_write(0x00);
        }
        
        for (int i = 0; i < 40; i++) {
            __delay_ms(1);
        }
        
        
        
    }
}

// Main Interrupt Service Routine (ISR)
void interrupt ISR(void)
{
    if(PIR3bits.RXB0IF)
    {
        LATDbits.LATD0 ^= 1;
        Message newMessage;
        ecan_receive_rxb0(&newMessage);
        ecan_rxb0_clear();          // Clear flag
        if (temperature > 24.0) {
            uart_write(0x00);
        } else {
            uart_write(newMessage.data[0]);
        }
        
        
        PIR3bits.RXB0IF = 0;
    }
    /*if(PIR3bits.RXB1IF)
    {
        Message newMessage;
        ecan_receive_rxb1(&newMessage);
        ecan_rxb1_clear();
        char speed = newMessage.data[0];
        uart_write(speed);
        
        PIR3bits.RXB1IF = 0;
    }*/
}