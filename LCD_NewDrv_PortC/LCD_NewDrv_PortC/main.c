/* ######################################################
EDITS:
Removed 30ms timer in ADC to handle 8 pieces at once.
Commented out print statement in ADC to improve the ADC operating speed.
Added a 100ms to EX sensor to allow the pieces to be pushed off the belt properly.
Added stepper acceleration. Currently accelerates to 8ms pauses after 9 steps.
Adjusted 

PROBLEMS:
Stepper got caught half way through a turn once during a full test run.
    //Cause was that a piece hit the divider while spinning in the opposite direction.
Pause state does not stop sorting when the other interrupts are running.

TODO:
Change interrupts to put pause and rampdown as higher interrupts than the exit sensor

#######################################################*/

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
#include "LinkedQueue.h"

//Function initializations
void mTimer ();
void ADCsetup();
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void PWMsetup();        					//initializing PWM 
void INTsetup();                         //initializing interrupts
void STProtate(int direction, int steps, int pause); //initializing the rotate function
void STPinitialization(); //initializing the intialization function for the stepper motor
void STPsort();
void STPaccelerate(int direction);
void STPdeaccelerate(int direction);	

//ADC variables
volatile unsigned int ADC_result;				//Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 
volatile unsigned int ADCcurr = 2000; 	//ADC interupt variable
volatile unsigned int ADCmin = 2000; 	//ADC interupt variable
volatile unsigned int ADCdone = 2;
volatile unsigned int ADCcounter = 0;

//Flags!!
volatile unsigned int motorflag = 1;
volatile unsigned int printflag = 0;
volatile unsigned int dontprintflag = 0;
volatile unsigned int stepperflag = 0;
volatile unsigned int buttonpushed = 0;
volatile unsigned int rampdownflag = 0;
volatile unsigned int pauseflag = 0;
volatile unsigned int systemrunning = 0;
volatile unsigned int counter = 0;
volatile unsigned int counter2 = 0;

//Stepper variables
volatile unsigned int CP = 1;  //initializes Current Position and sets it to 1.
volatile unsigned int STPpos;	//0 = black, 1 = white, 2 = steel, 3 = aluminum
volatile unsigned int STP90steps;	//50 - # of accel and deaccel steps
volatile unsigned int STP180steps; //100 - # of accel and deaccel steps
volatile unsigned int STPpause = 8;		//The amount of time the rotate function waits between steps

//Calibration variables 
volatile unsigned int whiteHigh = 950;
volatile unsigned int steelHigh = 800;
volatile unsigned int aluminumHigh = 400;

//Printing variables
volatile unsigned int SORTED = 0;
volatile unsigned int BLACK = 0;
volatile unsigned int WHITE = 0;
volatile unsigned int STEEL = 0;
volatile unsigned int ALUMINUM = 0;

link *head;				// The ptr to the head of the queue 
link *tail;				// The ptr to the tail of the queue 
link *newLink;			// Creates a newlink used for storing new items into the list

int main(int argc,char*argv[])
{

	ATOMIC_BLOCK(ATOMIC_FORCEON){
        InitLCD(LS_BLINK|LS_ULINE); //Initialize LCD module
	    LCDClear();  				//Clear the screen
		LCDWriteStringXY(0,0,"Starting...");
		mTimer(1000);
     }

    DDRA = 0xFF;            //PORT A set to outputs for the LEDs
    DDRB = 0xFF;           	//PORT B set to outputs for the pwm and DC motor)
	DDRD = 0b10110000;      //PORT D set to inputs for specific interrupts & buttons
	DDRE = 0xFF;			//PORT E set to outputs for the stepper motor
    DDRF = 0x00;           	//PORT F set to inputs (for the RL/ADC)

	STP90steps = 50 - (2 * STPpause);	//adjusts the # of steps based on the accl steps
	STP180steps = 100 - (2 * STPpause);	//adjusts the # of steps based on the accl steps
  

    cli();	//disables all interrupts
    ADCsetup();	//sets up the ADC
    INTsetup(); //sets up the interrupts
	PWMsetup();	//sets up the PWM
	STPinitialization();	//homes stepper

	setup(&head, &tail);	//connects head and tail of the linked list

    ATOMIC_BLOCK(ATOMIC_FORCEON){
	    LCDClear();  //Clear the screen
		LCDWriteStringXY(0,0,"Ready to Run");
    }
    
	
	//Tester code for the stepper
/*
	PORTA = CP;
	
	mTimer(2000);              //Wait 2 seconds
	STPaccelerate(1);
	STProtate(1,84,8);          //Rotate the stepper 180 degrees
	STPdeaccelerate(1);

	mTimer(100);
	STPaccelerate(1);
	STProtate(1,34,8);          
	STPdeaccelerate(1);

	mTimer(100);
	STPaccelerate(1);
	STProtate(1,34,8);          
	STPdeaccelerate(1);

	mTimer(100);
	STPaccelerate(1);
	STProtate(1,184,8);          
	STPdeaccelerate(1);

	PORTA = CP;

	mTimer(2000);              //Wait 2 seconds
	STPaccelerate(0);
	STProtate(0,84,8);          //Rotate the stepper -180 degrees
	STPdeaccelerate(0);

*/

    //DCmotor(1,0); 				 //run motor CW    

    while(1){

        //Start and pause button
        /*
		if((PIND & 0x80) == 0){ 				//if button is pushed
			mTimer(30);							//wait 30ms
			//buttonflag = 1;
			if((PIND & 0x80) == 0) {			//if button is still pushed
                DCmotor(1,0); 					//run motor CW
                pauseflag = 0;
				//if(systemrunning == 1) pauseflag = 1;	//brake motor
			    //mTimer(500);
                //systemrunning = 1;
                ADCmin = 2000;	//reset ADCmin to high value
                PORTA = 0x80;	//set 7th  LED for troubleshooting
            }
			
		}*/


		//Tester code for sensors


      	
        
		//PORTA = 0x00;

		//Tester code for the linked list
		/*
		if(size(&head, &tail) == 4) {	//4 items are stored in list, 
			printflag = 1;		//print flag set
			//DCmotor(0,1);		//brake motor
 			PORTA = 0x0F; 		
		}*/
        
        if(systemrunning == 1){
			DCmotor(1,0); 	//run motor CW
            //dontprintflag = 0;
            PORTA = 0x80;	//set 7th  LED for troubleshooting\
			//EIMSK |= _BV(INT1);	//unmask INT1
			
        } 
        else if((pauseflag == 1) && (rampdownflag == 0)){
            systemrunning = 0;
            //printflag = 2;
            PORTA = 0x10;
        }    

		if((rampdownflag == 1) && (size(&head, &tail) == 0)) {
			printflag = 1;
            PORTA = 0x0E;
		}
        
        if(printflag == 2){
            ATOMIC_BLOCK(ATOMIC_FORCEON){

                SORTED = ALUMINUM + STEEL + WHITE + BLACK;

                DCmotor(0,1);		//brake high to Vcc
				
				LCDClear();
            
                LCDWriteIntXY(0,0,head->e.itemCode,4);
                LCDWriteIntXY(5,0,head->next->e.itemCode,4);
                LCDWriteIntXY(10,0,head->next->next->e.itemCode,4);
		        LCDWriteIntXY(0,1,head->next->next->next->e.itemCode,4);
                LCDWriteIntXY(5,1,head->next->next->next->next->e.itemCode,4);
                LCDWriteIntXY(10,1,head->next->next->next->next->next->e.itemCode,4);
                //LCDWriteIntXY(8,1,head->next->next->next->next->next->next->e.itemCode,4);
		        //LCDWriteIntXY(12,1,head->next->next->next->next->next->next->next->e.itemCode,4);

        		printflag = 3;

            }
        }

        if(printflag == 1){
                
			//if(!motorflag) DCmotor(0,1);		//brake high to Vcc 
			//mTimer(1000);
            ATOMIC_BLOCK(ATOMIC_FORCEON){

                SORTED = ALUMINUM + STEEL + WHITE + BLACK;

                DCmotor(0,1);		//brake high to Vcc
				
				LCDClear();
            	LCDWriteStringXY(0,0,"S");
                LCDWriteIntXY(1,0,SORTED,2);
                LCDWriteStringXY(4,0,"P");
                LCDWriteIntXY(5,0,counter,2);
                LCDWriteStringXY(8,0,"R");
                LCDWriteIntXY(9,0,counter2,2);
				LCDWriteStringXY(12,0,"Q");
                LCDWriteIntXY(13,0,size(&head, &tail),2);
                LCDWriteStringXY(0,1,"B");
                LCDWriteIntXY(1,1,BLACK,2);
				LCDWriteStringXY(4,1,"W");
                LCDWriteIntXY(5,1,WHITE,2);
                LCDWriteStringXY(8,1,"S");
                LCDWriteIntXY(9,1,STEEL,2);
				LCDWriteStringXY(12,1,"A");
                LCDWriteIntXY(13,1,ALUMINUM,2);
        		printflag = 0;
				//dontprintflag = 1;
                //mTimer(1000);
                //EIMSK |= _BV(INT1);	//unmask INT1
                
                if(rampdownflag) while(1); //STAY HERE FOREVER........

            }//end ATOMIC BLOCK

        }//end print
        		        
    }//end while


   return(0);

}

ISR(INT0_vect)	//Rampdown INT
{
    rampdownflag = 1;
    while((PIND & 0x01) == 0);
    EIMSK &= ~_BV(INT0);	//mask INT0
    counter2++;
    //cli();
}

ISR(INT1_vect)	//Pause INT
{
	
    
    if(systemrunning == 1){
        pauseflag = 1;
        printflag = 2;
		systemrunning = 0;
        PORTA = 0xC0;
        DCmotor(0,1);		//brake high to Vcc
		//mTimer(500);
        //EIMSK &= ~_BV(INT1);	//mask INT1
    }
    else if(systemrunning == 0){
		pauseflag = 0;
        systemrunning = 1;
        PORTA = 0x30;
		//mTimer(500);
		//EIMSK &= ~_BV(INT1);	//mask INT1
	}


	while((PIND & 0x02) == 0){
		PORTA = 0x0F;
	}  

    mTimer(500);  
    EIFR |= _BV(INTF1);
	
	//buttonpushed = 1;
    counter++;
}

ISR(INT2_vect)	//EX INT
{
    if((PIND & 0x04) == 0){
			DCmotor(0,1); //brake
			//PORTA = 0x10;
			//motorflag = 0;
			mTimer(55);
			
			STPsort();

			
            /*
			link *rtnLink;
			dequeue(&head, &tail, &rtnLink);
			free(rtnLink);
            */

			DCmotor(1,0); //run motor CW
			mTimer(100);

	}
}


ISR(INT3_vect) //OR sensor stops belt
{
    if(!((PIND & 0x08) == 0)){
        PORTA = 0x08;
        ADCcurr = 2000;
        ADCSRA |= _BV(ADSC);	//start ADC conversion
		//ADCcounter++;
		EIMSK &= ~_BV(INT3);	//mask INT3
    }
}

// the interrupt will be trigured if the ADC is done
ISR(ADC_vect)
{
    ADC_result = ADC;  //
    if(ADC_result < ADCcurr) ADCcurr = ADC_result;

    if(!((PIND & 0x08) == 0) || (ADCcounter < 375)) {
		ADCcounter++;
        ADCSRA |= _BV(ADSC);
        PORTA = 0x40;
    }
    else {
		
		initLink(&newLink);			//Create new list item
		newLink->e.itemCode = ADCcurr;	//Sets the current values of the two switches to the new list item
		enqueue(&head, &tail, &newLink);		    //Add the new item above to list

		//Calibration code
		/*
		ATOMIC_BLOCK(ATOMIC_FORCEON){ 
			LCDClear();
			LCDWriteStringXY(0,0,"P1:");
			LCDWriteIntXY(4,0,ADCcurr,4);
			LCDWriteStringXY(0,1,"ADC++:");
			LCDWriteIntXY(8,1,ADCcounter,4);
		}*/

        //printflag = 1; 
		ADCcounter = 0; 
		EIMSK |= _BV(INT3);	//unmask INT3
        PORTA = 0x20;
    }//end else

    //ADC_result_flag = 1;

}//end ADC INT

ISR(BADISR_vect)
{
	while(1) PORTA = 0b10101010;
}

void INTsetup()
{
	//cli(); //disables all interrupts
            // config the external interrupt
    EIMSK |= 0b00001111; // enable INT0 - INT3
	
    EICRA &= ~_BV(ISC01) & ~_BV(ISC00);	//low level for ramp button
    //EICRA |= _BV(ISC11);				//falling edge for pause button
	EICRA |= _BV(ISC21);				//falling edge on EX
    EICRA |= (_BV(ISC31) | _BV(ISC30)); // rising edge on OR
}


void PWMsetup() // Sets up the PWN - only needs to be called once 
{
    TCCR0A |= 0x83;             //Set Waveform Generator to mode to Fast PWM   
    //TIMSK0 = TIMSK0 | 0b00000010;
    TCCR0B = 0b00000010;        //Clock scaling by 1/8
    OCR0A = 0b10000000;         //Duty cycle set to ~50%
}

void ADCsetup()
{
    ATOMIC_BLOCK(ATOMIC_FORCEON){
                
        ADCSRA |= _BV(ADEN);
        ADMUX |= _BV(REFS0);
        ADMUX |= _BV(MUX0);
        //ADCSRA |= _BV(ADSC);
        ADCSRA |= _BV(ADIE);
    }
}

void DCmotor(int direction, int brakeHigh)  //direction 1 = CW, direction 0 = CCW, brakeHigh 1 = brake to DC
{
    if(brakeHigh){
        PORTB = ~0x0F;       //Brake DC motor to Vcc
    }
    else if(direction){
        PORTB = ~0x0B; 		//Run DC motor clockwise
    }
    else if(direction == 0){
        PORTB = ~0x07; 		//Run DC motor counter-clockwise
    }
}

void STPinitialization()
{
    int i = 0;

	while(i < 201)
	{
		if((PIND & 0x40) == 0) {	//Sensor testing code snippet
	        PORTA = 0x40;
			STPpos = 0;	//STP position set to black
			return;
	    }
	
		STProtate(1, 1, 20);	//rotate one step CW
		i++;
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
            PORTE = 0b00110000;     //motor A rotates (step 1)
            CP = 1;                 //Current Position set to 1
            }
            else if(CP == 1){
            PORTE = 0b00000110;     //motor B rotates (step 2)
            CP = 2;		    //Current Position set to 2
            }
            else if(CP == 2){
            PORTE = 0b00101000;     //motor A rotates (step 3)
            CP = 3;		    //Current Position set to 3
            }
            else if(CP == 3){
            PORTE = 0b00000101;     //motor B rotates (step 4)
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
            PORTE = 0b00101000;     //motor A rotates (step 3)
            CP = 3;		    //Current Position set to 3
            }
            else if(CP == 3){
            PORTE = 0b00000110;     //motor B rotates (step 2)
            CP = 2;		    //Current Position set to 2
            }
            else if(CP == 2){
            PORTE = 0b00110000;     //motor A rotates (step 1)
            CP = 1; 		    //Current Position set to 1
            }
            else if(CP == 1){
            PORTE = 0b00000101;     //motor B rotates (step 4)
            CP = 4;		    //Current Position set to 4
            }

            mTimer(pause);          //wait designated time
            i++;		    //Increment loop counter
        }//end while
    }//end else if

    return;
}//end STProtate function

//direction = 0 is backwards, direction = 1 is forwards
void STPaccelerate(int direction) //accelerates the motor in 8 steps
{
	STProtate(direction,1,20);
	STProtate(direction,1,19);
	//STProtate(direction,1,18);
	//STProtate(direction,1,17);
	STProtate(direction,1,16);
	//STProtate(direction,1,15);
	//STProtate(direction,1,14);
	STProtate(direction,1,13);
	//STProtate(direction,1,12);
	STProtate(direction,1,11);
	STProtate(direction,1,10);
	STProtate(direction,1,9);
	STProtate(direction,1,8);
}

//direction = 0 is backwards, direction = 1 is forwards
void STPdeaccelerate(int direction) //deaccelerates the motor in 8 steps
{
	STProtate(direction,1,8);
	STProtate(direction,1,9);
	STProtate(direction,1,10);
	STProtate(direction,1,11);
	//STProtate(direction,1,12);
	STProtate(direction,1,13);
	//STProtate(direction,1,14);
	//STProtate(direction,1,15);
	STProtate(direction,1,16);
	//STProtate(direction,1,17);
	//STProtate(direction,1,18);
	STProtate(direction,1,19);
	STProtate(direction,1,20);
}

void STPsort()
{
	int itemVal = head->e.itemCode;

	if(STPpos == 0)	//at black position
	{
		if(itemVal < aluminumHigh){
			STPaccelerate(1);
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);
			STPpos = 3;			//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(0);
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);
			STPpos = 2;			//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(1);
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);
			STPpos = 1;			//pos set to white
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
			STPaccelerate(0);
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);
			STPpos = 3;			//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(1);
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);
			STPpos = 2;			//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPpos = 1;			//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(1);
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);
			STPpos = 0;			//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end white pos condition

	else if(STPpos == 2) //at steel position
	{
		if(itemVal < aluminumHigh){
			STPaccelerate(1);
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);
			STPpos = 3;			//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPpos = 2;			//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(0);		
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);
			STPpos = 1;			//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(1);
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);
			STPpos = 0;			//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end steel pos condition

	else if(STPpos == 3) //at aluminum position
	{
		if(itemVal < aluminumHigh){
			STPpos = 3;			//pos set to alum
            ALUMINUM++;
			PORTA = 0x08;
		}
		else if(itemVal < steelHigh){
			STPaccelerate(1);
			STProtate(1,STP180steps,STPpause);	//rotate CW 180deg
			STPdeaccelerate(1);
			STPpos = 2;			//pos set to steel
            STEEL++;
			PORTA = 0x04;
		}
		else if(itemVal < whiteHigh){
			STPaccelerate(1);
			STProtate(1,STP90steps,STPpause);	//rotate CW 90deg
			STPdeaccelerate(1);
			STPpos = 1;			//pos set to white
            WHITE++;
			PORTA = 0x02;
		}
		else if(itemVal >= whiteHigh){
			STPaccelerate(0);
			STProtate(0,STP90steps,STPpause);	//rotate CCW 90deg
			STPdeaccelerate(0);
			STPpos = 0;			//pos set to black
            BLACK++;
			PORTA = 0x01;
		}
	}//end aluminum pos condition
}//end stepperSort

//direction = 1 is CW, and direction = 0 is CCW


void mTimer (int count)
{
   int i;
   i = 0;

   TCCR1B |= _BV (CS10);  //  sets prescalar to DIV 1
   // Set the Waveform gen. mode bit description to clear on compare mode only
   TCCR1B |= _BV(WGM12); // Set output compare register for 1000 cycles, 1ms
   
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
