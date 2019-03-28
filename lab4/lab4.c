/*########################################################################
# MILESTONE : 4
# PROGRAM : 4A
# PROJECT : Interfacing
# GROUP : X
# NAME 1 : Patrick, Church, V00802612
# NAME 2 : Riley, Confrey, V00817892
# DESC : This program initializes the stepper motor to turn CW 101 steps to
reach a particular position. From there the stepper motor turns CW 30°, then 60°, 
and then 180°. After that the stepper motor turns CCW 30°, then 60°, and then 180°.
# DATA
# REVISED

######################################################################## */

#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey

void delaynus(int n); //initializing the microsecond delay function
void delaynms(int n); //initializing the milisecond delay function
void mTimer(int count); //initializing the timer function
void rotate(int direction, int steps, int pause); //initializing the rotate function
void initializeStepper(); //initializing the intialization function for the stepper motor

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){

DDRA = 0xFF;            
TCCR1B |= _BV(CS10);    //Initialize the timer control register, clock scaling CS10              

	while(1){

		PORTA = 0x01;
		mTimer(2000);              //Wait 2 seconds
		PORTA = 0x02;
		mTimer(3000);              //Wait 3 seconds
	}

return (0); //This line returns a 0 value to the calling program
// generally means no error was returned
}//end main



void mTimer(int count)
{
    int i = 0;                    // Loop counter
    TCCR1B |= _BV(WGM12);         // Set to Waveform Generator Mode 12 (Clear on Timer Compare)
    OCR1A = 0x03E8;               // Set output compare register for 1000 cycles(1ms)
    TCNT1 = 0x0000;               // Set initial counter timer time to 0
    TIMSK1 = TIMSK1 | 0b00000010; // Enable output compare interrupt
    TIFR1 |= _BV(OCF1A);          // Clear interrupt flag and start timer

    //TODO: switch this to a real interrupt
    while (i < count)
    {
        //When the interrupt's flag is triggered
        if ((TIFR1 & 0x02) == 0x02)
        {
            TIFR1 |= _BV(OCF1A); //Clear the flag
            i++;                 //increment the loop
        }
    }
    return;
}
