/*
 * miniProject.c
 *
 *  Created on: Feb 10, 2025
 *      Author: Hesham
 */

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

#define BUZZER_ALERTS 10   //number of buzzer alerts when counting down to zero.

void timer1_CTC_init(void);
void EX_Interrupts_init(void);

typedef enum{COUNT_UP, COUNT_DWN}Timer_Mode;
typedef enum{COUNT_ON, COUNT_OFF}Timer_State;
Timer_Mode mode = COUNT_UP;
Timer_State state = COUNT_ON;

signed char seconds=0, minutes=0, hours=0;
char counter=0;  //to count buzzer alerts

// Reset PB
ISR(INT0_vect){
	seconds=0;
	minutes=0;
	hours=0;
}

// Pause PB
ISR(INT1_vect){
	state = COUNT_OFF;
	TIMSK  &= ~(1<<OCIE1A);
}

// Resume PB
ISR(INT2_vect){
	state = COUNT_ON;
	timer1_CTC_init();
}

ISR(TIMER1_COMPA_vect){
	if(mode == COUNT_UP){
		seconds++;
		if(seconds>=60){minutes++;seconds=0;}
		if(minutes>=60){hours++;minutes=0;}
		if(hours>=24)hours=0;
	}else{
		if(state == COUNT_ON){
			seconds--;
			if(seconds<0){minutes--;seconds=59;}
			if(minutes<0){hours--;minutes=59;}
			if(hours<0){
				state = COUNT_OFF;
				PORTD |= (1<<0); //operate buzzer
				seconds = 0;
				minutes = 0;
				hours=0;
				OCR1A  = 3906;; //set timer1 interrupt to 0.25s
				counter=0;
			}
		}else{ //Count_OFF state
			PORTD ^= (1<<0); // toggle buzzer
			counter++;
			if(counter >=BUZZER_ALERTS){
				PORTD &= ~(1<<0);
				TIMSK  &= ~(1<<OCIE1A);  //turn off Timer1
			}
		}
	}
}


int main(void){
	//initialize Timer1 and Ex. Interrupts
	SREG   |=(1<<7);
	timer1_CTC_init();
	EX_Interrupts_init();

	// setting first 6 pins of PORTA as outputs and reset them
	DDRA |= 0x3F;
	PORTC &= ~(0x3F);

	// setting whole PORTB as input and enable the PU resistor
	DDRB = 0x00;
	PORTB = 0xFF;

	// setting first 4 pins of PORTC as outputs and reset them
	DDRC |= 0x0F;
	PORTC &= ~(0x0F);

	// setting PD2 & PD3 as inputs, with PD2 has internal resistor
	// setting PD0, PD4, and PD5 as outputs and reset them except PD4
	DDRD &= ~(0x0C);
	DDRD |= (0x31);
	PORTD |= 0x14;
	PORTD &= ~(0x21);

	char MFlag  = 0;  //flag for up/down push button
	char SIFlag = 0;  //flag for seconds increment push button
	char SDFlag = 0;  //flag for seconds decrement push button
	char MIFlag = 0;  //flag for minutes increment push button
	char MDFlag = 0;  //flag for minutes decrement push button
	char HIFlag = 0;  //flag for hours increment push button
	char HDFlag = 0;  //flag for hours decrement push button

	while(1){
		//Display seconds
		PORTA = (PORTA & 0xC0) | (1<<5);
		PORTC = (PORTC & 0xF0) | ((seconds%10) & 0x0F);
		_delay_ms(1);
		PORTA = (PORTA & 0xC0) | (1<<4);
		PORTC = (PORTC & 0xF0) | ((seconds/10%10) & 0x0F);
		_delay_ms(1);

		//Display minutes
		PORTA = (PORTA & 0xC0) | (1<<3);
		PORTC = (PORTC & 0xF0) | ((minutes%10) & 0x0F);
		_delay_ms(1);
		PORTA = (PORTA & 0xC0) | (1<<2);
		PORTC = (PORTC & 0xF0) | ((minutes/10%10) & 0x0F);
		_delay_ms(1);

		//Display hours
		PORTA = (PORTA & 0xC0) | (1<<1);
		PORTC = (PORTC & 0xF0) | ((hours%10) & 0x0F);
		_delay_ms(1);
		PORTA = (PORTA & 0xC0) | (1<<0);
		PORTC = (PORTC & 0xF0) | ((hours/10%10) & 0x0F);
		_delay_ms(1);

		// Up/Down mode select PB
		if(!(PINB & (1<<7)) && state == COUNT_OFF && MFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<7))){
				PORTD ^= (0x30);
				mode ^=1;
				MFlag = 1;
			}
		}

		//the flag is written like this to guarantee not interrupting the seven segment display
		if((PINB & (1<<7)))MFlag=0;

		// Seconds increment PB
		if(!(PINB & (1<<6)) && state == COUNT_OFF && SIFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<6))){
				seconds++;
				if(seconds>=60){minutes++;seconds=0;}
				if(minutes>=60){hours++;minutes=0;}
				if(hours>=24)hours=0;
				SIFlag=1;
			}
		}
		if((PINB & (1<<6))) SIFlag=0;

		// Seconds decrement PB
		if(!(PINB & (1<<5)) && state == COUNT_OFF && SDFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<5))){
				seconds--;
				if(seconds<0){minutes--;seconds=59;}
				if(minutes<0){hours--;minutes=59;}
				if(hours<0)hours=23;
				SDFlag=1;
			}
		}
		if((PINB & (1<<5))) SDFlag=0;

		// Minutes increment PB
		if(!(PINB & (1<<4)) && state == COUNT_OFF && MIFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<4))){
				minutes++;
				if(minutes>=60){hours++;minutes=0;}
				if(hours>=24)hours=0;
				MIFlag=1;
			}
		}
		if((PINB & (1<<4))) MIFlag=0;

		// Minutes decrement PB
		if(!(PINB & (1<<3)) && state == COUNT_OFF && MDFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<3))){
				minutes--;
				if(minutes<0){hours--;minutes=59;}
				if(hours<0)hours=23;
				MDFlag=1;
			}
		}
		if((PINB & (1<<3))) MDFlag=0;

		// Hours increment PB
		if(!(PINB & (1<<1)) && state == COUNT_OFF && HIFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<1))){
				hours++;
				if(hours>=24)hours=0;
				HIFlag=1;
			}
		}
		if((PINB & (1<<1))) HIFlag=0;

		// Hours decrement PB
		if(!(PINB & (1<<0)) && state == COUNT_OFF && HDFlag == 0){
			_delay_ms(20);
			if(!(PINB & (1<<0))){
				hours--;
				if(hours<0)hours=23;
				HDFlag=1;
			}
		}
		if((PINB & (1<<0))) HDFlag=0;

	}
}

//function to initialize Timer1
void timer1_CTC_init(void){
	TCCR1A = (1<<FOC1A)|(1<<FOC1B);
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS12);
	TCNT1  = 0;
	OCR1A  = 15625;
	TIMSK  |=(1<<OCIE1A);
}

//function to initialize external interrupts
void EX_Interrupts_init(void){
	MCUCR = (1<<ISC01) | (1<<ISC10) | (1<<ISC11);
	MCUCSR |= (1<<ISC2);
	GICR |= (1<<INT0) | (1<<INT1) | (1<<INT2);
}


