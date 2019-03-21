#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey

// define the global variables that can be used in every function
volatile unsigned char ADC_result;				      //Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 

void ADCsetup() // Sets up the ADC - only needs to be called once 
{

    cli();  // disable all of the interrupt
            // config the external interrupt
    EIMSK |= (_BV(INT2)); // enable INT2
    EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt
        // config ADC
        // by default, the ADC input (analog input is set to be ADC0 / PORTF0
    ADCSRA |= _BV(ADEN); // enable ADC
    ADCSRA |= _BV(ADIE); // enable interrupt of ADC
    ADMUX |= (_BV(ADLAR) | _BV(REFS0)); 

}

ISR(INT2_vect)
{
    ADCSRA |= _BV(ADSC); // when there is a rising edge, we need to do ADC
}


// the interrupt will be trigured if the ADC is done
ISR(ADC_vect)
{
    ADC_result = ADCH;  //%%%%%%%%%%%%%%%%%%%%%%%%%%%
    ADC_result_flag = 1;
}
