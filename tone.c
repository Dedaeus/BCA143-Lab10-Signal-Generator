#include <platform.h>
#include <math.h>
#include <dac.h>
#include <adc.h>
#include <timer.h>
#include <gpio.h>
#include "tone.h"
#include "delay.h"
#include "queue.h"
#include "main.h"

#define NUM_STEPS 64
#define QUEUE_SIZE 16
#define PI 3.1415927

/* RGB LED macros - PF8=Blue, PF9=Red, PF10=Green (active low) */
#define LED_ALL_OFF()  do { \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_SET); \
} while(0)

#define LED_RED()  do { \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8|GPIO_PIN_10, GPIO_PIN_SET);  \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET);            \
} while(0)

#define LED_GREEN()  do { \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);   \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);           \
} while(0)

#define LED_BLUE()  do { \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_SET);  \
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);            \
} while(0)

int sine_table[NUM_STEPS];
Queue play_queue;

void timer_callback_isr(void);
void sinewave_init(void);

void tone_init(void) {
    queue_init(&play_queue, QUEUE_SIZE);
    dac_init();
    timer_set_callback(timer_callback_isr);
    sinewave_init();
    gpio_set_mode(P_SW, PullUp);
    LED_ALL_OFF();
}

void tone_play_with_busy_waiting(int period_us, int num_cycles, wavetype wave) {
    int sample;
    int step;
    while(num_cycles-- > 0) {
        for (step = 0; step < NUM_STEPS; step++) {
            switch(wave) {
                case SINE:
                    sample = sine_table[step];
                    break;
                case SQUARE:
                    sample = step < NUM_STEPS / 2 ? 0 : DAC_MASK;
                    break;
                case RAMP:
                    sample = (step * DAC_MASK) / NUM_STEPS;
                    break;
            }
            // Simulate interference - wait until switch is released
            while (gpio_get(P_SW)) ;

            dac_set(sample);
            delay_us(period_us);
        }
    }
}

void tone_play_with_interrupt(int period_us, int num_cycles, wavetype wave) {
    int sample;
    int step;
    timer_init((period_us * (CLK_FREQ / 1000000UL)) / NUM_STEPS);
    timer_enable();
    while(num_cycles-- > 0) {
        for (step = 0; step < NUM_STEPS; step++) {
            switch(wave) {
                case SINE:
                    sample = sine_table[step];
                    break;
                case SQUARE:
                    sample = step < NUM_STEPS / 2 ? 0 : DAC_MASK;
                    break;
                case RAMP:
                    sample = (step * DAC_MASK) / NUM_STEPS;
                    break;
            }
            // Simulate interference - wait until switch is released
            while (!gpio_get(P_SW)) ;

            // Keep trying to add sample even if queue is full
            while (!queue_enqueue(&play_queue, sample));

            // GREEN = queue is full
            if (queue_is_full(&play_queue)) {
                LED_GREEN();
            }
        }
    }
    timer_disable();
}

void timer_callback_isr(void) {
    int sample;
    if (queue_dequeue(&play_queue, &sample)) {
        dac_set(sample);
        // Check queue state after dequeue
        if (queue_is_empty(&play_queue)) {
            LED_RED();    // RED = empty (underflow risk)
        } else {
            LED_BLUE();   // BLUE = between empty and full
        }
    } else {
        // Queue was empty when ISR fired = underflow
        LED_RED();
    }
}

void sinewave_init(void) {
    int n;
    for (n = 0; n < NUM_STEPS; n++) {
        sine_table[n] = (int)((DAC_MASK) * (1 + sin(n * 2 * PI / NUM_STEPS)) / 2);
    }
}
