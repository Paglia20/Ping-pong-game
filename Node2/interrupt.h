#ifndef CAN_INTERRUPT_H_
#define CAN_INTERRUPT_H_

#define IR_ADC_CH      7          // A0 is PA16 -> ADC7
#define IR_ADC_CH_MASK (1u << IR_ADC_CH)

void ADC_Handler(void);
void ir_adc_init(void);
extern volatile uint8_t ir_beam_blocked;


#endif /* CAN_INTERRUPT_H_ */