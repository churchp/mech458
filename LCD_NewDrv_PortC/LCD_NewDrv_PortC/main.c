/* ######################################################
EDITS:
Removed 30ms timer in ADC to handle 8 pieces at once.
Commented out print statement in ADC to improve the ADC operating speed.
Added a 100ms to EX sensor to allow the pieces to be pushed off the belt properly.
Added stepper acceleration. Currently accelerates to 8ms pauses after 8 steps.
Adjusted delay before turning the stepper so that pieces don't land on the edges.
Print statements added for sorted, queued, and # of each element sorted.
Rampdown button added on INT0.
Pause button added on INT1.
Added mTimer2 to correct rampdown state.

PROBLEMS:


TODO:
Speed up belt?
Add stacking?
Add look-ahead? Then pre-turn the stepper?
#######################################################*/

#include <stdlib.h> //the header of the general purpose standard library of C programming language
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdfix.h>
#include <util/delay.h>
#include <avr/interrupt.h>  // Delay functions for AT90USBKey
#include <util/atomic.h>

#include "lcd.h"
#include "LinkedQueue.h"

//Function initializations
void mTimer();   //initializing the general timer
void mTimer2();  //initializing the interrupt timer for rampdown
void ADCsetup(); //initializing the ADC
void PWMsetup(); //initializing the PWM 
void INTsetup(); //initializing interrupts
void STPsetup(); //initializing stepper motor
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void STProtate(int direction, int steps, int pause); //initializing the stepper rotate function
void STPaccelerate(int direction);		//initializing the stepper acceleration function
void STPdeaccelerate(int direction);	//initializing the stepper deacceleration function
void STPsort();	 //initializing the stepper sorting algorithm

//ADC variables
volatile unsigned int ADC_result;				//Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 
volatile unsigned int ADCmin = 2000; 			//ADC low value variable
volatile unsigned int ADCcounter = 0;			//Stores the number of ADC conversions while 1 piece passes RL

//Flags!!
volatile unsigned int printflag = 0;	//Used to switch activate the print code once rampdown or pause has been initialized
volatile unsigned int rampdownflag = 0;	//Used to switch into rampdown state
volatile unsigned int pauseflag = 0;	//Used to switch into pause state
volatile unsigned int systemrunning = 0;//Used to switch into run state

//Stepper variables
volatile unsigned int CP = 1;   //Current Position for the stepper set to 1.
volatile unsigned int STPpos;	//Used in the stepper algorithm to tell which quadrant is facing the belt
								//0 = black, 1 = white, 2 = steel, 3 = aluminum
volatile unsigned int STP90steps;	//Used to turn the stepper 90 degrees
									//50 - # of accel and deaccel steps
volatile unsigned int STP180steps; 	//Used to turn the stepper 180 degrees
									//100 - # of accel and deaccel steps
volatile unsigned int STPpause = 8;	//The amount of time the rotate function waits between steps

//Calibration variables 
volatile unsigned int whiteHigh = 930;
volatile unsigned int steelHigh = 800;
volatile unsigned int aluminumHigh = 175;

//Printing variables
volatile unsigned int SORTED = 0;	//Stores the number of sorted items
volatile unsigned int BLACK = 0;	//Stores the number of black sorted items
volatile unsigned int WHITE = 0;	//Stores the number of white sorted items
volatile unsigned int STEEL = 0;	//Stores the number of steel sorted items
volatile unsigned int ALUMINUM = 0;	//Stores the number of aluminum sorted items

link *head;		//The ptr to the head of the queue 
link *tail;		//The ptr to the tail of the queue 
link *newLink;	//Creates a newlink used for storing new items in the list

int main(int argc,char*argv[])
{

	ATOMIC_BLOCK(ATOMIC_FORCEON){
        	InitLCD(LS_BLINK|LS_ULINE); //Initialize LCD module
		LCDClear();  				//Clear the screen
		LCDWriteStringXY(0,0,"Starting...");
		mTimer(1000);
     	}

    DDRA = 0xFF;            	//PORT A set to outputs for the LEDs
    DDRB = 0xFF;           	//PORT B set to outputs for the pwm and DC motor)
    DDRD = 0b10110000;      	//PORT D set to inputs for specific interrupts & buttons
    DDRE = 0xFF;		//PORT E set to outputs for the stepper motor
    DDRF = 0x00;           	//PORT F set to inputs (for the RL/ADC)

	STP90steps = 50 - (2 * STPpause);	//adjusts the # of steps based on the accl steps
	STP180steps = 100 - (2 * STPpause);	//adjusts the # of steps based on the accl steps
  

    	cli();		//disables all interrupts
    	ADCsetup();	//sets up the ADC
    	INTsetup(); 	//sets up the interrupts
	PWMsetup();	//sets up the PWM
	STPsetup();	//homes the stepper

	setup(&head, &tail);	//connects head and tail of the linked list

    ATOMIC_BLOCK(ATOMIC_FORCEON){
	LCDClear();  //Clear the screen
	LCDWriteStringXY(0,0,"Ready to Run");
    }  

    while(1){
        
	//As long as systemrunning is true the program will loop through here
        if(systemrunning == 1){
		DCmotor(1,0); 	//run motor CW
            	PORTA = 0x80;	//LEDs for troubleshooting	
        } 
	//Once pause is called, as long as rampdown has not been called the program will loop through here
        else if((pauseflag == 1) && (rampdownflag == 0)){
            systemrunning = 0;	//reset systemrunning to zero so that the system stays in pause state
            PORTA = 0x10;	//LEDs for troubleshooting	
        }    

	//Once rampdown is called the system must wait until the queue is empty
	if((rampdownflag == 1) && (size(&head, &tail) == 0)) {
		printflag = 1;	//Set to 1 to call the printing code
            	PORTA = 0x0E;	//LEDs for troubleshooting	
	}
        
        if(printflag == 1){
                
		    ATOMIC_BLOCK(ATOMIC_FORCEON){

				SORTED = ALUMINUM + STEEL + WHITE + BLACK;

				DCmotor(0,1);	//brake high to Vcc
				
				LCDClear();		//Clears the screen
				LCDWriteStringXY(0,0,"Sorted:");
				LCDWriteIntXY(7,0,SORTED,2);	//Displays the number of sorted items
				LCDWriteStringXY(11,0,"Q:");
				LCDWriteIntXY(13,0,size(&head, &tail),2);	//Displays the number of items in the queue
				LCDWriteStringXY(0,1,"B");
				LCDWriteIntXY(1,1,BLACK,2);		//Displays the number of black sorted items
				LCDWriteStringXY(4,1,"W");
				LCDWriteIntXY(5,1,WHITE,2);		//Displays the number of white sorted items
				LCDWriteStringXY(8,1,"S");
				LCDWriteIntXY(9,1,STEEL,2);		//Displays the number of steel sorted items	
				LCDWriteStringXY(12,1,"A");
				LCDWriteIntXY(13,1,ALUMINUM,2);	//Displays the number of aluminum sorted items
				printflag = 0;	//Stops the program from constantly re-printing
				
				if(rampdownflag) while(1); //STAY HERE FOREVER........

            }//end ATOMIC BLOCK

        }//end print
        		        
    }//end while


   return(0);

}//end main

ISR(INT0_vect)	//Rampdown interrupt
{ 
    while((PIND & 0x01) == 0);	//Wait until the rampdown button is released
	mTimer2();		//Call mTimer2 for 2 seconds
    	EIMSK &= ~_BV(INT0);	//mask INT0
}

ISR(INT1_vect)	//Start and pause interrupt
{
    if(systemrunning == 1){
        pauseflag = 1;		//Set to pause state
        printflag = 1;		//Set printflag to call the printing code once
	systemrunning = 0;	//Disable run state
        PORTA = 0xC0;		//LEDs for troubleshooting
        DCmotor(0,1);		//brake high to Vcc
    }
    else if(systemrunning == 0){
	pauseflag = 0;		//Disable pause state
        systemrunning = 1;	//Set to run state
        PORTA = 0x30;		//LEDs for troubleshooting
    }

    while((PIND & 0x02) == 0){	//Wait until button has been released
	PORTA = 0x0F;		//LEDs for troubleshooting
    }  

    mTimer(500);  
    EIFR |= _BV(INTF1);	//
}

ISR(INT2_vect)	//Exit sensor interrupt
{
    if((PIND & 0x04) == 0){
		DCmotor(0,1); 	//Brake the DC motor
		mTimer(55);	//Wait 
			
		STPsort();	//Calls the sorting algorithm to position the tray correctly
            
		link *rtnLink;	//creates a temporary link to help dequeue
		dequeue(&head, &tail, &rtnLink);	//dequeues the first link in the queue
		free(rtnLink); 	//frees up the memory of "rtnLink" 

		DCmotor(1,0); 	//run motor CW
		mTimer(100);	//Delay for 0.1s so that the current piece is pushed off the belt before the next piece reaches the exit sensor
	}
}


ISR(INT3_vect) //OR sensor stops belt
{
    if(!((PIND & 0x08) == 0)){
        PORTA = 0x08;
        ADCmin = 2000;		//Reset ADCmin to well above the minimum value of a black piece
        ADCSRA |= _BV(ADSC);	//Start ADC conversion
	EIMSK &= ~_BV(INT3);	//Mask INT3
    }
}

// the interrupt will be trigured if the ADC is done
ISR(ADC_vect)
{
    ADC_result = ADC;  
    if(ADC_result < ADCmin) ADCmin = ADC_result;

    if(!((PIND & 0x08) == 0) || (ADCcounter < 375)) {
	ADCcounter++;
        ADCSRA |= _BV(ADSC);	//Starts another ADC conversion
        PORTA = 0x40;
    }
    else {
	initLink(&newLink);		//Creates a new list item
	newLink->e.itemCode = ADCmin;	//Sets the minimum value of the ADC to the new list item code
	enqueue(&head, &tail, &newLink);//Adds the new item to the queue

	ADCcounter = 0; 
	EIMSK |= _BV(INT3);	//Unmask INT3
        PORTA = 0x20;
    }//end else

}//end ADC INT

ISR(TIMER3_COMPA_vect)
{
	rampdownflag = 1;
}

ISR(BADISR_vect)
{
	while(1) PORTA = 0b10101010;
}

void INTsetup()
{
    //Configures the external interrupts
    EIMSK |= 0b00001111; //Enable INT0 - INT3
	
    EICRA &= ~_BV(ISC01) & ~_BV(ISC00);	//Low level for ramp button
    EICRA &= ~_BV(ISC11) & ~_BV(ISC10);	//Low level for pause button
    EICRA |= _BV(ISC21);		//Falling edge on EX
    EICRA |= (_BV(ISC31) | _BV(ISC30)); //Rising edge on OR
}

void ADCsetup()
{
    ATOMIC_BLOCK(ATOMIC_FORCEON){         
        ADCSRA |= _BV(ADEN);
	ADCSRA |= _BV(ADIE);
        ADMUX |= _BV(REFS0);
        ADMUX |= _BV(MUX0);
    }
}

void PWMsetup() // Sets up the PWN - only needs to be called once 
{
    TCCR0A |= 0x83;             //Set Waveform Generator to mode to Fast PWM   
    TCCR0B = 0b00000010;        //Clock scaling by 1/8
    OCR0A = 0b10000000;         //Duty cycle set to 50%
}

void STPsetup()
{
    int i = 0;
	while(i < 201)
	{
		if((PIND & 0x40) == 0) {//If the hall effect sensor is tripped
	        PORTA = 0x40;
			STPpos = 0;		//Stepper position set to black
			return;			//Stepper has homed correctly. Exit function.
	    }
	
		STProtate(1, 1, 20);//rotate one step CW
		i++;
	}
}

//direction 1 = CW, direction 0 = CCW, brakeHigh 1 = brake to DC
void DCmotor(int direction, int brakeHigh)  
{
    if(brakeHigh){
        PORTB = ~0x0F;	//Brake DC motor to Vcc
    }
    else if(direction){
        PORTB = ~0x0B; 	//Run DC motor clockwise
    }
    else if(direction == 0){
        PORTB = ~0x07; 	//Run DC motor counter-clockwise
    }
}

//direction = 0 is backwards, direction = 1 is forwards
//steps = # of steps
//pause = amount of time in ms between steps
void STProtate(int direction, int steps, int pause)
{
    int i = 0;		//Initialize loop counter

    if(direction)	//Executes if the direction is clockwise
    {
        while(i < steps)
        {
            if(CP == 4){
            PORTE = 0b00110000; //motor A rotates (step 1)
            CP = 1;             //Current Position set to 1
            }
            else if(CP == 1){
            PORTE = 0b00000110; //motor B rotates (step 2)
            CP = 2;		    	//Current Position set to 2
            }
            else if(CP == 2){
            PORTE = 0b00101000; //motor A rotates (step 3)
            CP = 3;		    	//Current Position set to 3
            }
            else if(CP == 3){
            PORTE = 0b00000101; //motor B rotates (step 4)
            CP = 4;		    	//Current Position set to 4
            }

            mTimer(pause);  //wait designated time
            i++;		    //Increment loop counter
        } //end while
    }//end if

    else if(!direction)	//Executes if the direction is counter-clockwise
    {
        while(i < steps)
        {
            if(CP == 4){
            PORTE = 0b00101000; //motor A rotates (step 3)
            CP = 3;		    	//Current Position set to 3
            }
            else if(CP == 3){
            PORTE = 0b00000110; //motor B rotates (step 2)
            CP = 2;		    	//Current Position set to 2
            }
            else if(CP == 2){
            PORTE = 0b00110000; //motor A rotates (step 1)
            CP = 1; 		    //Current Position set to 1
            }
            else if(CP == 1){
            PORTE = 0b00000101; //motor B rotates (step 4)
            CP = 4;		    	//Current Position set to 4
            }

            mTimer(pause);  //wait designated time
            i++;		    //Increment loop counter
        }//end while
    }//end else if

    return;
}//end STProtate function

//direction = 0 is CCW, direction = 1 is CW
void STPaccelerate(int direction) //accelerates the motor in 8 steps
{
	STProtate(direction,1,20);
	STProtate(direction,1,19);
	STProtate(direction,1,16);
	STProtate(direction,1,13);
	STProtate(direction,1,11);
	STProtate(direction,1,10);
	STProtate(direction,1,9);
	STProtate(direction,1,8);
}

//direction = 0 is CCW, direction = 1 is CW
void STPdeaccelerate(int direction) //deaccelerates the motor in 8 steps
{
	STProtate(direction,1,8);
	STProtate(direction,1,9);
	STProtate(direction,1,10);
	STProtate(direction,1,11);
	STProtate(direction,1,13);
	STProtate(direction,1,16);
	STProtate(direction,1,19);
	STProtate(direction,1,20);
}

void STPsort()
{
	int itemVal = head->e.itemCode;

	if(STPpos == 0)	//at black position
	{
		if(itemVal < aluminumHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 3;		//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(0);	//Accelerate CCW
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);	//Deaccelerate CCW
			STPpos = 2;		//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 1;		//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPpos = 0;			//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end black pos condition

	else if(STPpos == 1) //at white position
	{
		if(itemVal < aluminumHigh){
			STPaccelerate(0);	//Accelerate CCW
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);	//Deaccelerate CCW
			STPpos = 3;		//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 2;		//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPpos = 1;		//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 0;		//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end white pos condition

	else if(STPpos == 2) //at steel position
	{
		if(itemVal < aluminumHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 3;		//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPpos = 2;		//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(0);	//Accelerate CCW	
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);	//Deaccelerate CCW
			STPpos = 1;		//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 0;		//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end steel pos condition

	else if(STPpos == 3) //at aluminum position
	{
		if(itemVal < aluminumHigh){
			STPpos = 3;		//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 2;		//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(1);	//Accelerate CW
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);	//Deaccelerate CW
			STPpos = 1;		//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(0);	//Accelerate CCW
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);	//Deaccelerate CCW
			STPpos = 0;		//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end aluminum pos condition
}//end STPsort




void mTimer (int count)
{
   int i = 0;

   TCCR1B |= _BV (CS10); //Sets prescalar to DIV 1
   TCCR1B |= _BV(WGM12); //Set the Waveform gen. mode bit description to clear on compare mode only
   
   OCR1A = 0x03E8; 	// Set output compare register for 1000 cycles, 1ms
   TCNT1 = 0x0000; 	//Initialize Timer 1 to zero
   TIFR1 |= _BV(OCF1A); //Clear the timer interrupt flag and begin timing
   
   //Poll the timer to determine when the timer has reached 1ms
   while (i < count)
   {
      while ((TIFR1 & 0x02) != 0x02);
	   //Clear the interrupt flag by WRITING a ONE to the bit
	   TIFR1 |= _BV(OCF1A);
	   i++;
   }
   TCCR1B &= ~_BV (CS10);  // disable prescalar
   return;
}//end mTimer


void mTimer2()
{
   TCCR3B |= _BV (CS30) | _BV (CS32)|  _BV(WGM32);  //Sets prescalar to DIV 1024
   //Set the Waveform gen. mode bit description to clear on compare mode only
   
   TIMSK3 |= _BV(OCIE3A);  //Enable the output compare interrupt 
	
   TCNT3 = 0x0000; //Initialize Timer 1 to zero  
   OCR3A = 0x07A1; //set timer for 2s 
   TIFR3 |= _BV(OCF3A); //Clear the timer interrupt flag and begin timing 
 
   return;
}//end mTimer2
