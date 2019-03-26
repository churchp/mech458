/*########################################################################

TURNS BELT ON WITH BUTTON

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
    //ADCsetup(); // calls ADCsetup function to ready the ADC for use 
    buttonSetup(); //calls buttonsetup

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




// the interrupt will be trigured if the ADC is done



