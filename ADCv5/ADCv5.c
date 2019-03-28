/*########################################################################

TURNS BELT ON, TURNS BELT OFF WITH OR (OPTICAL REFLECTIVITY SENSOR)

########################################################################*/

#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey


volatile unsigned int ADC_result;				//Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable
volatile unsigned int motorFlag = 0;
volatile unsigned int STATE = 1;


void mTimer(int count); 					//initializing the timer function
void PWMsetup();        					//initializing PWM 
void PWM();             					//initializing PWM function
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void ADCsetup();       						//initializing ADC
void INTsetup();                         //initializing button

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){

    DDRA = 0xFF;            //lights
 	DDRD = 0b11110111;		//interrupt input on PIND3
    DDRB = 0xFF;           //Sets all PORT B pins to output (for the pwm and motor)
    TCCR1B |= _BV(CS10);   //Initialize the timer control register, clock scaling CS10              

	cli();

    //PWMsetup(); // calls PWMsetup function to ready the PWM for use 
	INTsetup(); //calls INTsetup

    sei(); // sets the Global Enable for all interrupts
	
    //ADCsetup(); // calls ADCsetup function to ready the ADC for use 
    

    while (1) //Main while loop
    {

		switch(STATE)
		{
			case(0): //idle state
				
				break;
			case(1):	//belt runnning state
				if(motorFlag != 1){
					DCmotor(1,0); 			//run motor CW
					motorFlag = 1;
					PORTA = 0x01;
				}
				break;
			case(2):	//piece has reached the OR sensor
				//if(motorFlag != 0){
					DCmotor(0,1);		//brake high to Vcc
					motorFlag = 0;
					PORTA = 0x02;
					mTimer(30);
				//}
				break;
			case(3):	//ADC is working, then output ADC result
			
				break;	
		}
    }


return (0); //This line returns a 0 value to the calling program
            // generally means no error was returned
}


void INTsetup()
{
	//cli(); //disables all interrupts
            // config the external interrupt
    EIMSK |= 0b00001000; // enable INT1, INT3
    EICRA |= _BV(ISC31) | _BV(ISC30); // rising edge interrupt
}

void PWMsetup() // Sets up the PWN - only needs to be called once 
{
    TCCR0A |= 0x83;             //Set Waveform Generator to mode to Fast PWM   
    TIMSK0 = TIMSK0 | 0b00000010;
    TCCR0B = 0b00000010;        //Clock scaling by 1/8
    OCR0A = 0b10000000;         //Duty cycle set to 50%
}

void ADCsetup() // Sets up the ADC - only needs to be called once 
{
    //cli();	//disables all interrupts
	        // config the external interrupt
    EIMSK |= (_BV(INT2)); // enable INT2
    EICRA |= (_BV(ISC21) | _BV(ISC20)); // rising edge interrupt
        // config ADC
        // by default, the ADC input (analog input is set to be ADC0 / PORTF0
    ADCSRA |= _BV(ADEN); // enable ADC
    ADCSRA |= _BV(ADIE); // enable interrupt of ADC
    ADMUX |= (_BV(REFS0)); 

}

void PWM(int dutyCycle) 
{
    OCR0A = dutyCycle;
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

ISR(INT3_vect) //OR sensor stops belt
{
    STATE = 2;
}
/*
ISR(ADC_vect)
{
    ADC_result = ADC; 
    ADC_result_flag = 1;
	STATE = 3;
}
/*
ISR(BADISR_vect)
{
	DCmotor(0,1);   //brake high (to Vcc)
	PORTA = 0xAA;
}
*/
void mTimer(int count)
{
	cli();
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
	sei();
    return;
}






