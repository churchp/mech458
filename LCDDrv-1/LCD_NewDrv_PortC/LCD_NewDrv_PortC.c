#include <avr/io.h>
#include <util/delay.h>
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey

#include "lcd.h"


volatile unsigned char ADC_result;				      //Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 



void mTimer(int count); 
void ADCsetup();  

int main(int argc,char*argv[])
{
	unsigned int i = 1212;

    //Initialize LCD module
	InitLCD(LS_BLINK|LS_ULINE);

	//Clear the screen
	LCDClear();

	//Simple string printing
	LCDWriteIntXY(4,0,i,4);
	//LCDWriteStringXY(12,1,"%");


    DDRA = 0xFF;    //Sets all PORT A pins to output (for the LEDs)
    DDRF = 0x00;    //Sets all PORT F pins to input (for the potentiometer)

    sei(); // sets the Global Enable for all interrupts
    ADCSRA |= _BV(ADSC); // initialize the ADC, start one conversion at the beginning

    PORTA = 0x0F;

     while (1) //Main while loop
    {
        

        if (ADC_result_flag)
        {
            PORTA = ADC_result;     //display ADC result to the LCD display
            ADC_result_flag = 0x00; //reset ADC flag
        }

    }
	

   return(0);

}


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

void mTimer (int count)
{
   int i;
   i = 0;
   TCCR1B |= _BV (CS10);  //  sets prescalar to DIV 1
   /* Set the Waveform gen. mode bit description to clear
     on compare mode only */
   TCCR1B |= _BV(WGM12);
   /* Set output compare register for 1000 cycles, 1ms */
   OCR1A = 0x03E8;
   /* Initialize Timer 1 to zero */
   TCNT1 = 0x0000;
   /* Enable the output compare interrupt */
   //TIMSK1 |= _BV(OCIE1A);  //remove if global interrups is set (sei())
   /* Clear the timer interrupt flag and begin timing */
   TIFR1 |= _BV(OCF1A);
   /* Poll the timer to determine when the timer has reached 1ms */
   while (i < count)
   {
      while ((TIFR1 & 0x02) != 0x02);
	
	   /* Clear the interrupt flag by WRITING a ONE to the bit */
	   TIFR1 |= _BV(OCF1A);
	   i++;
   } /* while */
   TCCR1B &= ~_BV (CS10);  //  disable prescalar
   return;
}  /* mTimer */
