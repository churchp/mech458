/*########################################################################
# MILESTONE : 1
# PROGRAM : 1
# PROJECT : MyFirstProject
# GROUP : X
# NAME 1 : Patrick, Church, V00802612
# NAME 2 : Riley, Confrey, V00817892
# DESC : This program outputs a different configuration
 of bits after every delay of roughly 200ms. These configurations of bits will display a
 Knight Rider affect on a set of 8 LEDs.
# DATA
# REVISED

######################################################################## */

#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h> // Delay functions for AT90USBKey
#include "LinkedQueue.c"

void delaynus(int n); //initializing the microsecond delay function
void delaynms(int n); //initializing the milisecond delay function
void mTimer(int count);

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){


DDRC = 0b11111111; /* Sets all pins on Port D to output */
DDRA = 0b00000000; /* Sets all pins on Port A to input */
DDRD = 0b00000000; /* Sets all pins on Port E to input */

TCCR1B |= _BV(CS10); // Initalize the timer control register, clock scaling CS01

	link *head;			/* The ptr to the head of the queue */
	link *tail;			/* The ptr to the tail of the queue */
	link *newLink;
setup(&head, &tail);
int flag = 0;


while(1)
{
int i = (PIND & 0x80);
  
if(i == 0) { 
    mTimer(30);
    PORTC = 0x01;
    flag = 1;     
}

if(i == 0x80 && flag == 1) 
{ 
       mTimer(30); 
       initLink(&newLink);
       newLink->e.itemCode = (PINA & 0b00000011);
	   enqueue(&head, &tail, &newLink);
       flag = 0;
       PORTC = 0x00;
}
    

if(size(&head, &tail) == 4) break;

}//while

PORTC = 0b00001100;
mTimer(1000);
PORTC = head->e.itemCode;

/*while(1){

if(i == 0) {
mTimer(30); 
flag = 1} 

if(i == 0x80 && flag == 1){mTimer(30);}

PORTC = head->e.itemCode;

}*/
return (0); //This line returns a 0 value to the calling program
// generally means no error was returned
}


void mTimer(int count)
{
    int i = 0;                    //loop counter
    TCCR1B |= _BV(WGM12);         // Set to Waveform Generator Mode 12 (Clear on Timer Compare)
    OCR1A = 0x03E8;               // Set output compare register for 1000 cycles(1ms)
    TCNT1 = 0x0000;               // Set initalal counter timer time to 0
    TIMSK1 = TIMSK1 | 0b00000010; // Enable output compare interrupt enable
    TIFR1 |= _BV(OCF1A);          // Clear interrupt flag and start timer

    //TODO: switch this to a real interrupt
    while (i < count)
    {
        //When the interrups flag is triggered
        if ((TIFR1 & 0x02) == 0x02)
        {
            TIFR1 |= _BV(OCF1A); //Clear the flag
            i++;                 //increment the loop
        }
    }
    return;
}


