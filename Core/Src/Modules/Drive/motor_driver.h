
#include "ugv_cmsis_interface.h"

void initialise_motor_pwm() {
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	HAL_TIM_PWM_Start(pwm_timer_handle, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(pwm_timer_handle, TIM_CHANNEL_2);
}

void set_left_motor_speed(uint16_t speed) {
	TIM3->CCR1 = speed;
}


void set_right_motor_speed(uint16_t speed) {
	TIM3->CCR2 = speed;
}
