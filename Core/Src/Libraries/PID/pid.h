
#ifndef PID_LIBRARY
#define PID_LIBRARY

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	float kp;
	float ki;
	float kd;
	float integral;

	float integral_limit;
	float error_previous;

	float min;
	float max;
} PID_t;

void pid_init(PID_t *pid);

int pid_set_parameters(PID_t *pid, float kp, float ki, float kd, float integral_limit, float min, float max);

float pid_calculate(PID_t *pid, float sp, float val, float val_dot, float dt);

void pid_reset_integral(PID_t *pid);

float clamp (float val, float min, float max);

#ifdef __cplusplus
}
#endif

#endif
