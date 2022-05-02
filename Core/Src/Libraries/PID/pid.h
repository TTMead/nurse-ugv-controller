

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
