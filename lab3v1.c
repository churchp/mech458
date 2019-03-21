/*########################################################################
# MILESTONE : 3
# PROGRAM : 3
# PROJECT : Linked Lists & MCU I/O
# GROUP : X
# NAME 1 : Patrick, Church, V00802612
# NAME 2 : Riley, Confrey, V00817892
# DESC : This program stores a two bit entry in a FIFO linked list every time
 a button is pushed, up to 4 entries. Once the max number of entries is reached 
 the first entry is dequeued then each subsequent entry is displayed 2 seconds 
 apart and shifted to the left by 2 LEDS.
# DATA
# REVISED

######################################################################## */

#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey
#include "LinkedQueue.c"    //The LinkedQueue file and the the attached header file

void delaynus(int n); //initializing the microsecond delay function
void delaynms(int n); //initializing the milisecond delay function
void mTimer(int count); //initializing the timer function

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){


DDRC = 0b11111111; /* Sets all pins on Port D to output */
DDRA = 0b00000000; /* Sets all pins on Port A to input */
DDRD = 0b00000000; /* Sets all pins on Port E to input */

TCCR1B |= _BV(CS10); 		// Initalize the timer control register, clock scaling CS01

	link *head;				/* The ptr to the head of the queue */
	link *tail;				/* The ptr to the tail of the queue */
	link *newLink;
    setup(&head, &tail);	//Creates new list
    int flag = 0;			//flag variable - used to keep track of when button has been pressed


while(1)
{
	int i = (PIND & 0x80); 			//button press variable - "0" when button is being pressed, "1" when not
  
	if(i == 0) { 					//if statement waiting for button press 
		mTimer(30);					//debouncing timer  
		flag = 1;     				// set flag variable to 1
        //PORTC = 0x01;				//turns on light when button is pressed
	}

	if(i == 0x80 && flag == 1){  	//if the button is not curently being pressed and has been pressed (flag=1) add item to list
		mTimer(30); 				//30ms delay
		initLink(&newLink);			//Create new list item
		newLink->e.itemCode = (PINA & 0b00000011);	//Sets the current values of the two switches to the new list item
		enqueue(&head, &tail, &newLink);		    //Add the new item above to list
		flag = 0;					//button released, flag to zero
		//PORTC = 0x00;				//turn off button indicator light
	}

    if(size(&head, &tail) == 4) break;  //waits for when the list has 4 items, then displays list and deletes all items in the list			

}//end while


link *rtnLink;
int outputBuffer = 0x00;

    dequeue(&head, &rtnLink);   //dequeue the first item from the list

    outputBuffer = head->e.itemCode;    //store the new first item in the buffer
	PORTC = outputBuffer;               //display the buffer

	mTimer(2000);                       //wait 2 seconds
    outputBuffer = outputBuffer + 0x04*(head->next->e.itemCode);
        //multiplies the 2nd item by 4 (to shift the bits left by 2) then adds that result to the buffer
	PORTC = outputBuffer;               //display the buffer
	
	mTimer(2000);                       //wait 2 seconds
    outputBuffer = outputBuffer + 0x10*(head->next->next->e.itemCode);
      //multiplies the 2nd item by 16 (to shift the bits left by 4) then adds that result to the buffer
	PORTC = outputBuffer;               //display the buffer

	clearQueue(&head, &tail);           //clear the queue
    free(rtnLink);                      


return (0); //This line returns a 0 value to the calling program
// generally means no error was returned

}//end main


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
