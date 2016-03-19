// acceleration test with led
// illuminates an LED connected to a gpio pin with gpio library functions
// use expansion header P8, pin8
// GPIO2_3 designated as gpio 67
#include <robotics_cape.h>
#define PIN 67
#define PIN2 66
#define ACCELERATION_CONSTANT_YAW 3.5
#define ACCELERATION_CONSTANT_PITCH 2
#define STEP_SIZE_YAW 0.00785398163 //radians

int32_t goal_position[2]; //pan, tilt
int32_t current_position[2]; //pan, tilt
int32_t steps_to_move[2]; //pan, tilt; negative or positive to determine direction-->negative=cc, postive=c


int micro_acceleration_delay(double step_theta, double acceleration_constant, int step_number);
void calculate_steps_to_move(int32_t *goal_position_cpy, int32_t *current_position_cpy);

int main (void){

// export gpio pin for use
if(gpio_export(PIN)){
	printf("Unable to open export.\n");
	return -1;
}

// set pin for output
if(gpio_set_dir(PIN, OUTPUT_PIN)){
	printf("Unable to open gpio67_direction.\n");
	return -1;
}

if(gpio_export(PIN2)){
	printf("Unable to open export.\n");
	return -1;
}

// set pin for output
if(gpio_set_dir(PIN2, OUTPUT_PIN)){
	printf("Unable to open gpio67_direction.\n");
	return -1;
}

int direction = 0;

goal_position[0] = 0;
goal_position[1] = 0;
current_position[0] = 0;
current_position[1] = 0;


while(true)
{

	printf("input goal position, 0-800:");
	scanf("%d", &goal_position[0]);
	
	calculate_steps_to_move(goal_position, current_position);

	int delay = micro_acceleration_delay(STEP_SIZE_YAW, ACCELERATION_CONSTANT_YAW, abs(steps_to_move[0]/2));

	if(delay<650)
		delay = 650;

	printf("%d", steps_to_move[0]);

	if(current_position[0] != goal_position[0])
		{
			if(steps_to_move[0] > 0)
			{
				
				gpio_set_value(PIN2, 1);
				gpio_set_value(PIN, 1);
					//	printf("ON\n");
				usleep(delay);
				// turn pin off
				gpio_set_value(PIN, 0);
				current_position[0]++;		
			}
			if(steps_to_move[0] < 0)
			{
				gpio_set_value(PIN2, 0);
				gpio_set_value(PIN, 1);
					//	printf("ON\n");
				usleep(delay);
				// turn pin off
				gpio_set_value(PIN, 0);
				current_position[0]++;		
				current_position[0]--;	
			}
			
			if(current_position[0] < 0)
				current_position[0] = 1600 + current_position[0];
			if(current_position[0] > 1600)
				current_position[0] = current_position[0] - 1600;
				
			usleep(delay);
		}



sleep(3);
}
return 1;
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
	if(steps_to_move[0] > 800)
	{
		steps_to_move[0] = steps_to_move[0] - 160;
	}
	if(steps_to_move[0] < -800)
	{
		steps_to_move[0] = steps_to_move[0] + 1600;
	}
	
	steps_to_move[1] = goal_position_cpy[1] - current_position_cpy[1];
	
}