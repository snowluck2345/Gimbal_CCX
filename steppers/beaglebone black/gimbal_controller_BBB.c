#include <robotics_cape.h>



//CONSTANTS
#define YAW_PULSE_PIN 67
#define YAW_DIRECTION_PIN 66
#define PITCH_PULSE_PIN 65
#define PITCH_DIRECTION_PIN 27
#define ACCELERATION_CONSTANT_YAW 3.5
#define ACCELERATION_CONSTANT_PITCH 20
#define STEP_SIZE_YAW 0.00785398163 //radians
#define STEP_SIZE_PITCH 0.0039269908 //radians

//GLOBAL DECLARATIONS
int32_t goal_position[2]; //pan, tilt
int32_t current_position[2]; //pan, tilt
int32_t steps_to_move[2]; //pan, tilt; negative or positive to determine direction-->negative=cc, postive=c

//FUNCTION DECLARATIONS
int micro_acceleration_delay(double step_theta, double acceleration_constant, int step_number);
void calculate_steps_to_move(int32_t *goal_position_cpy, int32_t *current_position_cpy);
void* yaw_move(void* arguement);
void* pitch_move(void* arguement);
int gpio_setup(void);

int main (void){

	int setup_success = gpio_setup();

	int direction = 0;
	int acceleration_flag;
	goal_position[0] = 0;
	goal_position[1] = 0;
	current_position[0] = 0;
	current_position[1] = 0;

	pthread_t yaw_thread;
	pthread_t pitch_thread;

	

	while(true)
	{
		printf("input goal position, 0-800:");
		scanf("%d", &goal_position[0]);
		goal_position[0] = goal_position[0] % 800;
		printf("input goal position, 0-800:");
		scanf("%d", &goal_position[1]);
		goal_position[1] = goal_position[1] % 800;
	
		calculate_steps_to_move(goal_position, current_position);

		printf("Steps to move: %d\n", steps_to_move[0]);
		printf("Steps to move: %d\n", steps_to_move[1]);

		int ret = pthread_create(&yaw_thread, NULL, yaw_move, NULL);
		if(ret<0) { printf("Thread Creation Failed\n");  return 1; }
		ret = pthread_create(&pitch_thread, NULL, pitch_move, NULL);
		if(ret<0) { printf("Thread Creation Failed\n");  return 1; }

		pthread_join(yaw_thread,NULL);
		pthread_join(pitch_thread,NULL);
		
	}
}

int micro_acceleration_delay(double step_theta, double acceleration_constant, int step_number)
{
	//step_theta is theta per step or microstep
	//acceleration constant is radians/s^2
	//step_number is delay calculation for the step I am interested in calculating

	double delay_seconds = sqrt(2*step_theta/acceleration_constant) * (sqrt(step_number + 1) - sqrt(step_number));
	int delay_microseconds = (int)(delay_seconds * 1000000);

	return delay_microseconds;
}

void calculate_steps_to_move(int32_t *goal_position_cpy, int32_t *current_position_cpy)
{
	steps_to_move[0] = goal_position_cpy[0] - current_position_cpy[0];
	if(steps_to_move[0] > 400)
	{
		steps_to_move[0] = steps_to_move[0] - 800;
	}
	if(steps_to_move[0] < -400)
	{
		steps_to_move[0] = steps_to_move[0] + 800;
	}
	
	steps_to_move[1] = goal_position_cpy[1] - current_position_cpy[1];
	
}

int gpio_setup(void)
{
	if(gpio_export(YAW_PULSE_PIN)){
		printf("Unable to open export.\n");
		return -1;
	}

	// set pin for output
	if(gpio_set_dir(YAW_PULSE_PIN, OUTPUT_PIN)){
		printf("Unable to open yaw_pulse_pin_direction.\n");
		return -1;
	}

	if(gpio_export(YAW_DIRECTION_PIN)){
		printf("Unable to open export.\n");
		return -1;
	}

	// set pin for output
	if(gpio_set_dir(YAW_DIRECTION_PIN, OUTPUT_PIN)){
		printf("Unable to open yaw_direction_pin_direction.\n");
		return -1;
	}

	if(gpio_export(PITCH_PULSE_PIN)){
		printf("Unable to open export.\n");
		return -1;
	}

	// set pin for output
	if(gpio_set_dir(PITCH_PULSE_PIN, OUTPUT_PIN)){
		printf("Unable to open pitch_pulse_pin_direction.\n");
		return -1;
	}

	if(gpio_export(PITCH_DIRECTION_PIN)){
		printf("Unable to open export.\n");
		return -1;
	}

	// set pin for output
	if(gpio_set_dir(PITCH_DIRECTION_PIN, OUTPUT_PIN)){
		printf("Unable to open pitch_direction_pin_direction.\n");
		return -1;
	}

	return 0;
}

void *yaw_move(void *arguement)
{
	int i = 0;
	int acceleration_flag = 0;

	while(current_position[0] != goal_position[0])
		{
			
			if(i <= abs(steps_to_move[0]/2) && acceleration_flag == 0)
			{
				i++;
			}
			else
				if(i > 0)
				{
					i--;
					acceleration_flag = 1;
				}
			int delay = micro_acceleration_delay(STEP_SIZE_YAW, ACCELERATION_CONSTANT_YAW, i);
			if(delay < 650)
				delay = 650;
			printf("yaw: %d, %d, %d, %d, %d\n", delay, i, steps_to_move[0], current_position[0], goal_position[0]);

			if(steps_to_move[0] > 0)
			{
				current_position[0]++;
				gpio_set_value(YAW_DIRECTION_PIN, 0);
				gpio_set_value(YAW_PULSE_PIN, 1);
				usleep(50);
				gpio_set_value(YAW_PULSE_PIN, 0);
			}
			if(steps_to_move[0] < 0)
			{
				current_position[0]--;
				gpio_set_value(YAW_DIRECTION_PIN, 1);
				gpio_set_value(YAW_PULSE_PIN, 1);
				usleep(50);
				gpio_set_value(YAW_PULSE_PIN, 0);
			}
			if(current_position[0] < 0)
				current_position[0] = 800 + current_position[0];
			if(current_position[0] >= 800)
				current_position[0] = current_position[0] - 800;
			usleep(delay);
		}

	i = 0;
	acceleration_flag = 0;
	pthread_exit(0);
	return 0;
}

void* pitch_move(void* arguement)
{
	int i = 0;
	int acceleration_flag = 0;

	while(current_position[1] != goal_position[1])
		{
			
			if(i <= abs(steps_to_move[1]/2) && acceleration_flag == 0)
			{
				i++;
			}
			else
				if(i > 0)
				{
					i--;
					acceleration_flag = 1;
				}
			int delay = micro_acceleration_delay(STEP_SIZE_PITCH, ACCELERATION_CONSTANT_PITCH, i);
			if(delay < 650)
				delay = 650;
			printf("pitch: %d, %d, %d, %d, %d\n", delay, i, steps_to_move[1], current_position[1], goal_position[1]);

			if(steps_to_move[1] > 0)
			{
				current_position[1]++;
				gpio_set_value(PITCH_DIRECTION_PIN, 0);
				gpio_set_value(PITCH_PULSE_PIN, 1);
				usleep(50);
				gpio_set_value(PITCH_PULSE_PIN, 0);
			}
			if(steps_to_move[1] < 0)
			{
				current_position[1]--;
				gpio_set_value(PITCH_DIRECTION_PIN, 1);
				gpio_set_value(PITCH_PULSE_PIN, 1);
				usleep(50);
				gpio_set_value(PITCH_PULSE_PIN, 0);
			}
			if(current_position[1] < 0)
				current_position[1] = 800 + current_position[1];
			if(current_position[1] >= 800)
				current_position[1] = current_position[1] - 800;
			usleep(delay);
		}

	i = 0;
	acceleration_flag = 0;
	pthread_exit(0);
	return 0;	
}