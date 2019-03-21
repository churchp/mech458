/* MILESTONE : 4
# PROGRAM : Part II
# PROJECT : Interfacing
# GROUP : X
# NAME 1 : Patrick, Church, V00802612
# NAME 2 : Riley, Confrey, V00817892
# DESC : 
# DATA
# REVISED: Feb25

########################################################################*/

#include <stdlib.h> // the header of the general purpose standard library of C programming language
#include <avr/io.h>// the header of i/o port
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey


// define the global variables that can be used in every function
volatile unsigned char ADC_result;				//Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 



void mTimer(int count); 					//initializing the timer function
void PWMsetup();        					//initializing PWM 
void PWM();             					//initializing PWM function
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void ADCsetup();       						//initializing ADC


//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){

    DDRA = 0xFF;           //Sets all PORT A pins to output (for the DC motor)
    DDRB = 0xFF;           //Sets all PORT B pins to output (for the pwm)
    DDRC = 0xFF;           //Sets all PORT C pins to output (for the LEDs)
    DDRE = 0x00;           //Sets all PORT E pins to input (for motor choices)
    DDRF = 0x00;           //Sets all PORT F pins to input (for the potentiometer)
    TCCR1B |= _BV(CS10);   //Initialize the timer control register, clock scaling CS10              

    PORTC = 0x00; // clear Port C

    PWMsetup(); // calls PWMsetup function to ready the PWM for use 
    ADCsetup(); // calls ADCsetup function to ready the ADC for use 

    sei(); // sets the Global Enable for all interrupts
    ADCSRA |= _BV(ADSC); // initialize the ADC, start one conversion at the beginning


    while (1) //Main while loop
    {

        if (ADC_result_flag)
        {
            PORTC = ADC_result;     //display ADC result to LEDs
            PWM(ADC_result);        //Change the duty cycle of the pwm
            ADC_result_flag = 0x00; //reset ADC flag
        }

        if((PINE & 0x03) == 0b00000010) 
        {
            if((PORTA & 0x0F) == ~0x07){ //if running CCW
                DCmotor(0,0);            //brake 
                mTimer(10);              //delay
            }
            DCmotor(1,0); 				 //run motor CW
        }

        else if((PINE & 0x03) == 0b00000001) 
        {
            if((PORTA & 0x0F) == ~0x0B){ //if running CW
                DCmotor(0,0);            //brake
                mTimer(10);              //delay
            }
            DCmotor(0,0); 				 //run motor CCW
        }

        else if((PINE & 0x03) == 0b00000011) 
        {
            DCmotor(0,1);   //brake high (to Vcc)
        }

        else DCmotor(0,1);  //brake high (to Vcc)

    }


return (0); //This line returns a 0 value to the calling program
            // generally means no error was returned
}



void DCmotor(int direction, int brakeHigh)  //direction 1 = CW, direction 0 = CCW, brakeHigh 1 = brake to DC
{
    if(brakeHigh){
        PORTA = ~0x0F;       //Brake DC motor to Vcc
    }
    else if(direction){
        PORTA = ~0x0B; 		//Run DC motor clockwise
    }
    else if(direction == 0){
        PORTA = ~0x07; 		//Run DC motor counter-clockwise
    }
}


void PWMsetup() // Sets up the PWN - only needs to be called once 
{
    TCCR0A |= 0x83;             //Set Waveform Generator to mode to Fast PWM   
    TIMSK0 = TIMSK0 | 0b00000010;
    TCCR0B = 0b00000010;        //Clock scaling by 1/8
    OCR0A = 0b10000000;         //Duty cycle set to 50%
           
   
}//end PWMsetup

void PWM(int dutyCycle) //%%%%%%%%%%%%%%%%%%%%%%
{
    OCR0A = dutyCycle;
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