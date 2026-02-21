/*
 * PWM.h
 *
 * Created: 12/02/2026 19:59:50
 *  Author: samur
 */ 


#ifndef PWM_H_
#define PWM_H_

//TIMER_0:
void PWM_T0_OC0A_PD6_OC0B_PD5(uint8_t AD6, uint8_t BD5, uint8_t WF_AD6, uint8_t WF_BD5, uint8_t MD, uint8_t TP, uint16_t PRS);
void LD_T0_OCR0A(uint8_t LD0A);
void LD_T0_OCR0B(uint8_t LD0B);

//TIMER_1:
void PWM_T1_OC1A_PB1_OC1B_PB2(uint8_t AB1, uint8_t BB2, uint8_t WF_AB1, uint8_t WF_BB2, uint8_t MD, uint8_t TP, uint16_t PRS);
void LD_T1_OCR1A(uint16_t LD1A);
void LD_T1_OCR1B(uint16_t LD1B);

void LD_T1_ICR1(uint16_t LD1ICR1);

//TIMER_2:
void PWM_T2_OC2A_PB3_OC2B_PD3(uint8_t AB3, uint8_t BD3, uint8_t WF_AB3, uint8_t WF_BD3, uint8_t MD, uint8_t TP, uint16_t PRS);
void LD_T2_OCR2A(uint8_t LD2A);
void LD_T2_OCR2B(uint8_t LD2B);

#endif /* PWM_H_ */