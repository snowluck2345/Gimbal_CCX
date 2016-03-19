/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>


//position of camera stage, based off a defined zero value with limit switches for pitch, starting value for yaw
volatile int yawCurrentPosSteps;
volatile int pitchCurrentPosSteps;

int yawGoalPosSteps;
int pitchGoalPosSteps;
int delay_stepping_ms[2][2000];

bool limitSwitch = false; //true means tripped, polled


//Initiation stuff
void position_init(void);
void serial_init(void);
void configure_port_pins(void);

//Battery stuff

//Position stuff
void home(void);
int goToPositionAbsolute(int *stepsFromHome);
void rampVelocity(void);
void pitchLimit(void);
void step(void);


int main (void)
{
	system_init();
	delay_init();
	configure_port_pins();
	position_init();
	//serial_init();
	
	//rampVelocity();	
	
	for(uint32_t i=0; i<50; i++)
	{
		
	}
	
	while (1) 
	{	
		step();
		delay_ms(50);
		//pitchLimit();
		
	}
	
	return 0;
}

void position_init(void)
{
	yawCurrentPosSteps = 0;
	home();
	pitchCurrentPosSteps = 0;
}

void serial_init(void)
{
	
}

void configure_port_pins(void)
{
	//configure outputs
	struct port_config config_port_pin;
	
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.input_pull = PORT_PIN_PULL_DOWN;
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(PIN_PB06, &config_port_pin);//yaw, pin 5 EXT1
	port_pin_set_config(PIN_PB07, &config_port_pin);//pitch, pin 6 EXT1
	
}

void home()
{
	//while(limitSwitch == false);
		{
			//make a step on pitch axis, not using a funcition	
		}
}

//call this function whenever you get a new position command, only once per command, it is not polled in the main loop
int goToPositionAbsolute(int *stepsFromHome) // returned: 0 return means moving, -1 means failed limit hit, 1 means successful, passs in an array for goal position, yaw=0th position, pitch=1st position
{
	int movementNumSteps[2];
	
	movementNumSteps[0]=yawGoalPosSteps - yawCurrentPosSteps;
	movementNumSteps[1]=pitchGoalPosSteps - pitchCurrentPosSteps;
	
	rampVelocity(); //determines step delays, 0th position=yaw, 1st position=pitch
	
	return 0;
}

void rampVelocity(void)
{
	for(int j = 0; j < 2; j++)
	{
		for(int i = 0; i < 40; i++)
		{
			delay_stepping_ms[j][i] = (41-i);
		}
		for(int i = 25; i < 375; i++)
		{
			delay_stepping_ms[j][i] = (2);
		}
		for(int i = 360; i < 400; i++)
		{
			delay_stepping_ms[j][i] = (i-368);
		}
	}
}


void pitchLimit(void)
{
	//if limit switch is tripped, or position pitch is too high, stop, neither condition should ever happen in normal operation
}

void step(void)
{
	//step yaw for 0, pitch for 1
	//implement later
	//reset delay counter in here and do checking if i should actually step--> make sure the delay has been long enough
	port_pin_set_output_level(PIN_PB06, true);
	delay_us(50);
	port_pin_set_output_level(PIN_PB06, false);
	
}