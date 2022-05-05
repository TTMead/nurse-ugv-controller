#include "pid.h"


float clamp (float val, float min, float max) {
	float output = val;

	if (val > max) {
		output = max;
	}
	if (output < min) {
		output = min;
	}

	return output;
}

void pid_init(PID_t *pid)
{
	pid->kp = 0.0f;
	pid->ki = 0.0f;
	pid->kd = 0.0f;
	pid->integral = 0.0f;
	pid->integral_limit = 0.0f;
	pid->error_previous = 0.0f;
	pid->min = 0.0f;
	pid->max = 0.0f;
}

 int pid_set_parameters(PID_t *pid, float kp, float ki, float kd, float integral_limit, float min, float max)
{
	 pid->kp = kp;
	 pid->ki = ki;
	 pid->kd = kd;
	 pid->integral_limit = integral_limit;
	 pid->min = min;
	 pid->max = max;
	 return 0;
}

float pid_calculate(PID_t *pid, float sp, float val, float val_dot, float dt)
{

	/* current error value */
	float error = sp - val;

	/* current error derivative */
	float error_dot = error - pid->error_previous;

	/* current integral */
	pid->integral = pid->integral + (error * dt);

	// Clamp integral to the limit
	pid->integral = clamp(pid->integral, pid->integral_limit, pid->integral_limit);



	/* calculate PID output */
	float output = (error * pid->kp) + (error_dot * pid->kd) + (pid->integral * pid->ki);

	/* limit output */
	// clamp(output, pid->min, pid->max);

	return output;
}


void pid_reset_integral(PID_t *pid)
{
	pid->integral = 0.0f;
}
