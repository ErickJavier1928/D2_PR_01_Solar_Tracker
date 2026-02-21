/*
 * PWM.c
 *
 * Created: 12/02/2026 20:00:06
 *  Author: samur
 */ 
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


//TIMER_0: AD6(ON(1)/OFF(0)), BD5(ON(1)/OFF(0)), WF_AD6(NI(0)/I(1)), WF_BD5(NI(0)/I(1)), MD(PC(0)/FS(1)), TP(0xFF(0)/(OCR0A|OCR0B)(1)), PRS(1-1024).
void PWM_T0_OC0A_PD6_OC0B_PD5(uint8_t AD6, uint8_t BD5, uint8_t WF_AD6, uint8_t WF_BD5, uint8_t MD, uint8_t TP, uint16_t PRS){
	//PORD: OUTPUT -> PD5 and PB6.
	if(AD6 == 1){
		DDRB |= (1<<DDD6);
	}
	else{}
	if(BD5 == 1){
		DDRB |= (1<<DDD5);
	}
	else{}
	
	//CL:
	TCCR1A = 0;
	TCCR1B = 0;
	
	//WF: NI -> 0 | I -> 1.
	if (WF_AD6 == 0){
		TCCR0A |= (1<<COM0A1);
	}
	else if (WF_AD6 == 1){
		TCCR0A |= (1<<COM0A0) |(1<<COM0A1);
	}
	else {}
	
	
	if (WF_BD5 == 0){
		TCCR0A |= (1<<COM0B1);
	}
	else if (WF_BD5 == 1){
		TCCR0A |= (1<<COM0B0) |(1<<COM0B1);
	}
	else {}
	
	//MODE: PC(0) and FS(1).
	if(MD == 0){
		//TP: 0xFF(0) and OCRA(1).
		if(TP == 0){
			TCCR0A |= (1<<WGM00);
		}
		else if (TP == 1){
			TCCR0A |= (1<<WGM00);
			TCCR0B |= (1<<WGM02);
		}
	}
	else if(MD == 1){
		//TP: 0xFF(0) and OCRA(1).
		if(TP == 0){
			TCCR0A |= (1<<WGM00) | (1<<WGM01);
		}
		else if (TP == 1){
			TCCR0A |= (1<<WGM00) | (1<<WGM01);
			TCCR0B |= (1<<WGM02);
		}
	}
	
	//T0_PRS: Default = 1.
	if(PRS == 1){
		TCCR0B |= (1<<CS00);
	}
	
	else if(PRS == 8){
		TCCR0B |= (1<<CS01);
	}
	
	else if(PRS == 64){
		TCCR0B |= (1<<CS00) | (1<<CS01);
	}
	
	else if(PRS == 256){
		TCCR0B |= (1<<CS02);
	}
	
	else if(PRS == 1024){
		TCCR0B |= (1<<CS00) | (1<<CS02);
	}
	
	else{
		TCCR0B |= (1<<CS00);
	}
	//OCR0A = 500;
	//OCR0B = 2400;
	
	
}
void LD_T0_OCR0A(uint8_t LD0A){
	//DUTY_CYCLE: PB2
	OCR0A = LD0A;
}
void LD_T0_OCR0B(uint8_t LD0B){
	//DUTY_CYCLE: PB2
	OCR0B = LD0B;
}

//TIMER_1:
void PWM_T1_OC1A_PB1_OC1B_PB2(uint8_t AB1, uint8_t BB2, uint8_t WF_AB1, uint8_t WF_BB2, uint8_t MD, uint8_t TP, uint16_t PRS){
	//PB1 and PB2:
	if(AB1 == 1){
		DDRB |= (1<<DDB1);
	}
	if(BB2 == 1){
		DDRB |= (1<<DDB2);
	}

	// Clear control registers
	TCCR1A = 0;
	TCCR1B = 0;

	//Fast mode:
	if(MD == 1){
		
		switch(TP){
			
			case 5: // Fast PWM 8-bit (TOP = 0x00FF)
			TCCR1A |= (1<<WGM10);
			TCCR1B |= (1<<WGM12);
			break;
			
			case 6: // Fast PWM 9-bit (TOP = 0x01FF)
			TCCR1A |= (1<<WGM11);
			TCCR1B |= (1<<WGM12);
			break;
			
			case 7: // Fast PWM 10-bit (TOP = 0x03FF)
			TCCR1A |= (1<<WGM10)|(1<<WGM11);
			TCCR1B |= (1<<WGM12);
			break;
			
			case 14: // Fast PWM, ICR1 as TOP
			TCCR1A |= (1<<WGM11);
			TCCR1B |= (1<<WGM12)|(1<<WGM13);
			break;
			
			case 15: // Fast PWM, OCR1A as TOP
			TCCR1A |= (1<<WGM10)|(1<<WGM11);
			TCCR1B |= (1<<WGM12)|(1<<WGM13);
			break;
			
			default: // Default: Fast PWM 8-bit
			TCCR1A |= (1<<WGM10);
			TCCR1B |= (1<<WGM12);
			break;
		}
	}
	
	//Phase correct default:
	else {
		switch(TP){
			
			case 1: // Phase Correct 8-bit (TOP = 0x00FF)
			TCCR1A |= (1<<WGM10);
			break;
			
			case 2: // Phase Correct 9-bit (TOP = 0x01FF)
			TCCR1A |= (1<<WGM11);
			break;
			
			case 3: // Phase Correct 10-bit (TOP = 0x03FF)
			TCCR1A |= (1<<WGM10)|(1<<WGM11);
			break;
			
			case 8: // Phase Correct, ICR1 as TOP
			TCCR1B |= (1<<WGM13);
			break;
			
			case 9: // Phase Correct, OCR1A as TOP
			TCCR1A |= (1<<WGM10);
			TCCR1B |= (1<<WGM13);
			break;
			
			case 10: // Phase Correct, ICR1 as TOP (WGM11+WGM13)
			TCCR1A |= (1<<WGM11);
			TCCR1B |= (1<<WGM13);
			break;
			
			case 11: // Phase Correct, OCR1A as TOP (WGM10+WGM11+WGM13)
			TCCR1A |= (1<<WGM10)|(1<<WGM11);
			TCCR1B |= (1<<WGM13);
			break;
			
			default: // Default: Phase Correct 8-bit
			TCCR1A |= (1<<WGM10);
			break;
		}
	}

	// Compare Output Mode (Non-inverting / Inverting)
	if (WF_AB1 == 0){
		TCCR1A |= (1<<COM1A1);
	}
	
	else {
		TCCR1A |= (1<<COM1A1)|(1<<COM1A0);
	}

	if (WF_BB2 == 0){
		TCCR1A |= (1<<COM1B1);
	}
	else {
		TCCR1A |= (1<<COM1B1)|(1<<COM1B0);
	}

	// Prescaler (kept as requested)
	if(PRS == 1){
		TCCR1B |= (1<<CS10);
	}
	else if(PRS == 8){
		TCCR1B |= (1<<CS11);
	}
	else if(PRS == 64){
		TCCR1B |= (1<<CS10)|(1<<CS11);
	}
	else if(PRS == 256){
		TCCR1B |= (1<<CS12);
	}
	else if(PRS == 1024){
		TCCR1B |= (1<<CS10)|(1<<CS12);
	}
	else{
		TCCR1B |= (1<<CS10);
	}

	// Default duty values
	//OCR1A = 2500;
	//OCR1B = 1250;
}
void LD_T1_OCR1A(uint16_t LD1A){
	//DUTY_CYCLE: PB2
	OCR1A = LD1A;
}
void LD_T1_OCR1B(uint16_t LD1B){
	//DUTY_CYCLE: PB2
	OCR1B = LD1B;
}

//If needed: ICR1 max instead of OCR1AB.
void LD_T1_ICR1(uint16_t LD1ICR1){
	ICR1 = LD1ICR1;
}

//TIMER_2: AB3(ON(1)/OFF(0)), BD3(ON(1)/OFF(0)), WF_AD7(NI(0)/I(1)), WF_BD3(NI(0)/I(1)), MD(PC(0)/FS(1)), TP(0xFF(0)/(OCR2A|OCR2B)(1)), PRS(1-1024).
void PWM_T2_OC2A_PB3_OC2B_PD3(uint8_t AB3, uint8_t BD3, uint8_t WF_AB3, uint8_t WF_BD3, uint8_t MD, uint8_t TP, uint16_t PRS){
	//PORTD: OUTPUT -> PB3 and PD3.
	if (AB3 == 1) {
		DDRD |= (1 << DDB3);
	}
	if (BD3 == 1) {
		DDRD |= (1 << DDD3);
	}

	//CL:
	TCCR2A = 0;
	TCCR2B = 0;

	//WF: NI -> 0 | I -> 1.
	if (WF_AB3 == 0) {
		TCCR2A |= (1 << COM2A1);
	}
	else if (WF_AB3 == 1) {
		TCCR2A |= (1 << COM2A0) | (1 << COM2A1);
	}

	if (WF_BD3 == 0) {
		TCCR2A |= (1 << COM2B1);
	}
	else if (WF_BD3 == 1) {
		TCCR2A |= (1 << COM2B0) | (1 << COM2B1);
	}

	//MODE: PC(0) and FS(1).
	if (MD == 0) {
		if (TP == 0) {
			TCCR2A |= (1 << WGM20);
		}
		else if (TP == 1) {
			TCCR2A |= (1 << WGM20);
			TCCR2B |= (1 << WGM22);
		}
	}
	else if (MD == 1) {
		if (TP == 0) {
			TCCR2A |= (1 << WGM20) | (1 << WGM21);
		}
		else if (TP == 1) {
			TCCR2A |= (1 << WGM20) | (1 << WGM21);
			TCCR2B |= (1 << WGM22);
		}
	}

	//T2_PRS: Default = 1.
	if (PRS == 1) {
		TCCR2B |= (1 << CS20);
	}
	else if (PRS == 8) {
		TCCR2B |= (1 << CS21);
	}
	else if (PRS == 32) {
		TCCR2B |= (1 << CS20) | (1 << CS21);
	}
	else if (PRS == 64) {
		TCCR2B |= (1 << CS22);
	}
	else if (PRS == 128) {
		TCCR2B |= (1 << CS20) | (1 << CS22);
	}
	else if (PRS == 256) {
		TCCR2B |= (1 << CS21) | (1 << CS22);
	}
	else if (PRS == 1024) {
		TCCR2B |= (1 << CS20) | (1 << CS21) | (1 << CS22);
	}
	else {
		TCCR2B |= (1 << CS20);
	}

	OCR2A = 128;
	OCR2B = 64;
}
void LD_T2_OCR2A(uint8_t LD2A){
	OCR2A = LD2A;
}
void LD_T2_OCR2B(uint8_t LD2B){
	OCR2B = LD2B;
}