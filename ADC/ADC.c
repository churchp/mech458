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
volatile unsigned int motorFlag = 0;



void mTimer(int count); 					//initializing the timer function
void PWMsetup();        					//initializing PWM 
void PWM();             					//initializing PWM function
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void ADCsetup();       						//initializing ADC
void buttonSetup();                         //initializing button

//################## MAIN ROUTINE ##################


int main(int argc, char *argv[]){

    DDRA = 0xFF;            //lights
    DDRF = 0xFF;           //Sets all PORT F pins to output (for the DC motor)
    DDRB = 0xFF;           //Sets all PORT B pins to output (for the pwm)
    DDRE = 0x00;           //Sets all PORT E pins to input (for motor choices)
    TCCR1B |= _BV(CS10);   //Initialize the timer control register, clock scaling CS10              

    PWMsetup(); // calls PWMsetup function to ready the PWM for use 
    ADCsetup(); // calls ADCsetup function to ready the ADC for use 

    sei(); // sets the Global Enable for all interrupts



    while (1) //Main while loop
    {



    }


return (0); //This line returns a 0 value to the calling program
            // generally means no error was returned
}


void DCmotor(int direction, int brakeHigh)  //direction 1 = CW, direction 0 = CCW, brakeHigh 1 = brake to DC
{
    if(brakeHigh){
        PORTF = ~0x0F;       //Brake DC motor to Vcc
    }
    else if(direction){
        PORTF = ~0x0B; 		//Run DC motor clockwise
    }
    else if(direction == 0){
        PORTF = ~0x07; 		//Run DC motor counter-clockwise
    }
}

void buttonSetup()
{
    cli();  // disable all of the interrupt
            // config the external interrupt
    EIMSK |= (_BV(INT1)); // enable INT1
    EICRA |= (_BV(ISC11) | _BV(ISC10)); // rising edge interrupt
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

ISR(INT1_vect) //motor controlled by button
{
    PORTA = 0x01;

    if(motorFlag) {
    DCmotor(0,1);   //brake high (to Vcc)
    motorFlag = 0;
    }
    else if (!motorFlag) {
    DCmotor(1,0); 			//run motor CW
    motorFlag = 1;
    }
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
