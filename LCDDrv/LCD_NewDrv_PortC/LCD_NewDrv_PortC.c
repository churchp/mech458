#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"


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



int main(int argc,char*argv[])
{
	unsigned int i = 11111;

	//Initialize LCD module
	InitLCD(LS_BLINK|LS_ULINE);

	//Clear the screen
	LCDClear();

	//Simple string printing
	LCDWriteIntXY(4,0,i,8);
	//LCDWriteStringXY(12,1,"%");


	

   return(0);

}



