#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdfix.h>
#include <util/delay.h>
#include <avr/interrupt.h>  // Delay functions for AT90USBKey
#include <util/atomic.h>

#include "lcd.h"


void mTimer ();
void ADCsetup();

volatile unsigned int ADC_result;				//Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 

char k;
unsigned int i;
//unsigned int j;

int main(int argc,char*argv[])
{
	DDRD = 0b11111011;
    ADCsetup();

    ATOMIC_BLOCK(ATOMIC_FORCEON){
	
        //Initialize LCD module
	    InitLCD(LS_BLINK|LS_ULINE);

	    //Clear the screen
	    LCDClear();  
    	
    }

    
   i = 10;

    while(1){
        
        mTimer(1000);
        
        ATOMIC_BLOCK(ATOMIC_FORCEON){
    	    
            i = ADC_result; 
            //i = i + 5;
            
        	LCDWriteStringXY(0,0,"ADC:");
            LCDWriteIntXY(6,0,i,4);
            //LCDWriteStringXY(0,1,"ADCL:");
            //LCDWriteIntXY(6,1,j,4);
            mTimer(200);	

        }
        
        

        if(ADC_result_flag) ADC_result_flag = 0;

    }


	

	

   return(0);

}



/*ISR(INT2_vect)
{

    ADCSRA |= _BV(ADSC); // when there is a rising edge, we need to do ADC

} */


// the interrupt will be trigured if the ADC is done
ISR(ADC_vect)

{
    ADCSRA |= _BV(ADSC);
    ADC_result = ADC;  //%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //ADC_result_flag = 1;

}





void ADCsetup()
{

    ATOMIC_BLOCK(ATOMIC_FORCEON){

        /*cli();  // disable all of the interrupt
                // config the external interrupt
        EIMSK |= (_BV(INT2)); // enable INT2
        EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt
            // config ADC
            // by default, the ADC input (analog input is set to be ADC0 / PORTF0
        ADCSRA |= _BV(ADEN); // enable ADC
        ADCSRA |= _BV(ADIE); // enable interrupt of ADC
        ADMUX |=  _BV(REFS0); */

        ADCSRA |= _BV(ADEN);
        ADMUX |= _BV(REFS0);
        ADMUX |= _BV(MUX0);
        ADCSRA |= _BV(ADSC);
        ADCSRA |= _BV(ADIE);

    }

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
