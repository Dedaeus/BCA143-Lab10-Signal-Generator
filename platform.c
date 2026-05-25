#include "platform.h"
#include "dac.h"
#include "timer.h"
#include "gpio.h"
#include "main.h"
#include "stm32f4xx_hal.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;

/* DAC */
void dac_init(void) {
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
}

void dac_set(int value) {
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1,
                     DAC_ALIGN_12B_R, value & DAC_MASK);
}

/* Timer */
static void (*_timer_cb)(void) = NULL;

void timer_set_callback(void (*cb)(void)) {
    _timer_cb = cb;
}

void timer_init(unsigned int period_cycles) {
    htim2.Init.Prescaler = 0;
    htim2.Init.Period = period_cycles - 1;
    HAL_TIM_Base_Init(&htim2);
}

void timer_enable(void) {
    HAL_TIM_Base_Start_IT(&htim2);
}

void timer_disable(void) {
    HAL_TIM_Base_Stop_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && _timer_cb != NULL) {
        _timer_cb();
    }
}

/* GPIO */
void gpio_set_mode(int pin, gpio_mode_t mode) {
    /* configured via CubeMX */
}

int gpio_get(int pin) {
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET) ? 1 : 0;
}
