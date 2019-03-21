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

volatile int CP = 1;  //initializes Current Position and sets it to 1.

void delaynus(int n); //initializing the microsecond delay function
void delaynms(int n); //initializing the milisecond delay function
void mTimer(int count); //initializing the timer function
void rotate(int direction, int steps, int pause); //initializing the rotate function
void initializeStepper(); //initializing the intialization function for the stepper motor

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){

DDRA = 0x3F;            //Sets the 1st 2 pins to input and the last 6 to output
TCCR1B |= _BV(CS10);    //Initialize the timer control register, clock scaling CS10              

initializeStepper(101);    //Initializes the stepper to end on a particular step

mTimer(3000);              //Wait 3 seconds
rotate(1,17,20);           //Rotate the stepper 30 degrees
mTimer(2000);              //Wait 2 seconds
rotate(1,33,20);           //Rotate the stepper 60 degrees
mTimer(2000);              //Wait 2 seconds
rotate(1,100,20);          //Rotate the stepper 180 degrees
mTimer(2000);              //Wait 2 seconds
rotate(0,17,20);           //Rotate the stepper -30 degrees
mTimer(2000);              //Wait 2 seconds
rotate(0,33,20);           //Rotate the stepper -60 degrees
mTimer(2000);              //Wait 2 seconds
rotate(0,100,20);          //Rotate the stepper -180 degrees



return (0); //This line returns a 0 value to the calling program
// generally means no error was returned
}//end main


void initializeStepper(int steps)
{
    rotate(1, steps, 20); //call the rotate function for a particular number of steps
}

//direction = 1 is CW, and direction = 0 is CCW
void rotate(int direction, int steps, int pause)
{
    int i = 0;		//Initialize loop counter

    if(direction)	//Executes if the direction is clockwise
    {
        while(i < steps)
        {
            if(CP == 4){
            PORTA = 0b00110000;     //motor A rotates (step 1)
            CP = 1;                 //Current Position set to 1
            }
            else if(CP == 1){
            PORTA = 0b00000110;     //motor B rotates (step 2)
            CP = 2;		    //Current Position set to 2
            }
            else if(CP == 2){
            PORTA = 0b00101000;     //motor A rotates (step 3)
            CP = 3;		    //Current Position set to 3
            }
            else if(CP == 3){
            PORTA = 0b00000101;     //motor B rotates (step 4)
            CP = 4;		    //Current Position set to 4
            }

            mTimer(pause);          //wait designated time
            i++;		    //Increment loop counter
        } //end while
    }//end if

    else if(!direction)	//Executes if the direction is counter-clockwise
    {
        while(i < steps)
        {
            if(CP == 4){
            PORTA = 0b00101000;     //motor A rotates (step 3)
            CP = 3;		    //Current Position set to 3
            }
            else if(CP == 3){
            PORTA = 0b00000110;     //motor B rotates (step 2)
            CP = 2;		    //Current Position set to 2
            }
            else if(CP == 2){
            PORTA = 0b00110000;     //motor A rotates (step 1)
            CP = 1; 		    //Current Position set to 1
            }
            else if(CP == 1){
            PORTA = 0b00000101;     //motor B rotates (step 4)
            CP = 4;		    //Current Position set to 4
            }

            mTimer(pause);          //wait designated time
            i++;		    //Increment loop counter
        }//end while
    }//end else if

    return;
}//end rotate

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
