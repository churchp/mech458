#include <avr/io.h>
#include <util/delay.h>
#include <util/delay_basic.h> //delay functions
#include <avr/interrupt.h>  // Delay functions for AT90USBKey

#include "lcd.h"


volatile unsigned char ADC_result;				      //Holds the most recent ADC value 
volatile unsigned int ADC_result_flag = 0x00; 	//ADC interupt variable 
volatile unsigned int motorFlag = 0;



void mTimer(int count); 					//initializing the timer function
void PWMsetup();        					//initializing PWM 
void PWM();             					//initializing PWM function
void DCmotor(int direction, int brakeHigh); //initializing DC motor function
void ADCsetup();       						//initializing ADC
void interruptSetup();                      //initializing button & OR  

int main(int argc,char*argv[])
{
	DDRA = 0xFF;            //lights
    DDRB = 0xFF;           //Sets all PORT B pins to output (for the pwm and motor)
    DDRE = 0x00;           //Sets all PORT E pins to input (for motor choices)
    TCCR1B |= _BV(CS10);   //Initialize the timer control register, clock scaling CS10              

    PWMsetup(); // calls PWMsetup function to ready the PWM for use 
    //ADCsetup(); // calls ADCsetup function to ready the ADC for use 
    interruptSetup(); //calls buttonsetup

    InitLCD(LS_BLINK|LS_ULINE);  //Initialize LCD module

    sei(); // sets the Global Enable for all interrupts
       
	
	LCDClear(); //Clear the screen

    //LCDWriteStringXY(0,0,"Start!");

    while (1) //Main while loop
    {
        

    }
	

   return(0);

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

void interruptSetup()
{
    cli();  // disable all of the interrupt
            // config the external interrupt
    EIMSK = 0b00001010; // enable INT1, INT3
    EICRA = 0b11001100; // rising edge interrupt
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

ISR(INT3_vect) //OR sensor stops belt
{
  //  LCDWriteStringXY(0,1,"Stop.");
    DCmotor(0,1);   //brake high (to Vcc)
    PORTA = 0x02;
    motorFlag = 0;
}

ISR(INT1_vect) //motor controlled by button
{
    

    if(motorFlag) {
    DCmotor(0,1);   //brake high (to Vcc)
    PORTA = 0x01;
    motorFlag = 0;
    }
    else if (!motorFlag) {
    DCmotor(1,0); 			//run motor CW
    motorFlag = 1;
    PORTA = 0x00;
    }
}

void mTimer (int count)
{
   int i;
   i = 0;
   TCCR1B |= _BV (CS10);  //  sets prescalar to DIV 1
   /* Set the Waveform gen. mode bit description to clear
     on compare mode only */
   TCCR1B |= _BV(WGM12);
   /* Set output compare register for 1000 cycles, 1ms */
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
