#include <stdio.h>
#include "sam/sam3x/include/sam.h"
#include "main.h"
#include "interrupt.h" 

volatile uint8_t  ir_beam_blocked = 0;     // 0 = beam present, 1 = blocked

static volatile uint16_t TH_HI = 3000;     // above this = "beam present" 
static volatile uint16_t TH_LO = 2600;     // below this = "beam blocked" 

static inline void adc_set_window(uint16_t low, uint16_t high) {
    if (high <= low) high = low + 1;       // keep a valid window
    ADC->ADC_CWR = ADC_CWR_LOWTHRES(low) | ADC_CWR_HIGHTHRES(high);
}

void ir_adc_init(void)
{
    // 1) Enable ADC peripheral clock
    PMC->PMC_PCER1 |= (1u << (ID_ADC - 32));

    //    ADC clock ~10.5 MHz (<=20 MHz), free-run, decent track/transfer/startup
    ADC->ADC_MR =
        ADC_MR_PRESCAL(3)       |   // ADCclk = MCK / ((PRESCAL+1)*2) -> 84/(4*2)=10.5 MHz
        ADC_MR_STARTUP_SUT64    |   // startup time
        ADC_MR_TRACKTIM(3)      |   // tracking cycles
        ADC_MR_TRANSFER(2)      |   // transfer cycles
        ADC_MR_FREERUN_ON;          // free-running conversions


    ADC->ADC_CHER = IR_ADC_CH_MASK; // 43.5.3

    // 4) Program window compare around your expected levels
    adc_set_window(TH_LO, TH_HI);

    // Compare on THIS channel, and trigger when sample is **outside** the window
    ADC->ADC_EMR =
        ADC_EMR_CMPSEL(IR_ADC_CH) |  // compare source channel
        ADC_EMR_CMPMODE_OUT;         // flag when value < LOW or > HIGH


    // 5) Enable interrupt on compare event i will uncomment this AFTER TESTING THRESHOLDS
    // ADC->ADC_IER = ADC_IER_COMPE;    // compare event
    // NVIC_EnableIRQ(ADC_IRQn);

    // 6) Start free-run conversions
    ADC->ADC_CR = ADC_CR_START;
}

void ADC_Handler(void)
{
    uint32_t isr = ADC->ADC_ISR;

    if (isr & ADC_ISR_COMPE) {
        uint16_t s = ADC->ADC_CDR[IR_ADC_CH] & 0x0FFF;  // 12-bit sample

        // Simple 2-level hysteresis:
        // - If we think beam is PRESENT (ir_beam_blocked==0) and we go BELOW TH_LO → BLOCKED
        // - If we think beam is BLOCKED (ir_beam_blocked==1) and we go ABOVE TH_HI → PRESENT
        if (!ir_beam_blocked) {
            if (s <= TH_LO) {
                ir_beam_blocked = 1;
                ball_count++;
            }
        } else {
            if (s >= TH_HI) {
                ir_beam_blocked = 0;
            }
        }
        ADC->ADC_CWR = ADC_CWR_LOWTHRES(TH_LO) | ADC_CWR_HIGHTHRES(TH_HI);
    }
}

//CMPMODES 
// OUT → interrupt when leaving window (both sides)
// IN → interrupt when inside window
// LOW → interrupt only when below low threshold
// HIGH → interrupt only when above high threshold


// what to do
// 1) check thresholds
// 2) enable interrupt
// 3) Test
//     if ok => approval
//     if not => try with debouncing 




//     volatile uint32_t sys_ms = 0;

//     void systick_init(void)
//     {
//         SysTick->LOAD  = (84000u - 1u);   // 84 MHz / 84,000 = 1 kHz tick (1 ms)
//         SysTick->VAL   = 0;
//         SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
//                         SysTick_CTRL_TICKINT_Msk |
//                         SysTick_CTRL_ENABLE_Msk;
//     }

//     void SysTick_Handler(void)
//     {
//         sys_ms++;
//     }

//     !! calling  in main:
//     SystemInit();
//     systick_init();

//     volatile uint32_t last_goal_time = 0;   // timestamp of last goal

//     void ADC_Handler(void)
//     {
//         uint32_t isr = ADC->ADC_ISR;

//         if (isr & ADC_ISR_COMPE) {
//             uint16_t s = ADC->ADC_CDR[IR_ADC_CH] & 0x0FFF;  // read ADC value

//             // Only detect transitions with hysteresis
//             if (!ir_beam_blocked) {
//                 // Check for beam break AND debounce
//                 if (s <= TH_LO && (sys_ms - last_goal_time > 300)) {  
//                     // 300 ms debounce window
//                     ir_beam_blocked = 1;
//                     ball_count++;
//                     last_goal_time = sys_ms;   // record time of valid event
//                 }
//             } else if (s >= TH_HI) {
//                 ir_beam_blocked = 0;           // beam restored
//             }

//             ADC->ADC_CWR = ADC_CWR_LOWTHRES(TH_LO) | ADC_CWR_HIGHTHRES(TH_HI);
//         }
//     }   