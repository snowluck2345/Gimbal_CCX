
#include <asf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define STEPS_PER_REVOLUTION 1600
#define MAX_RX_BUFFER_LENGTH 16



void usart_read_callback(const struct usart_module *const usart_module);
void usart_write_callback(const struct usart_module *const usart_module);
void configure_usart(void);
void configure_usart_callbacks(void);

void configure_port_pins(void);
void configure_tc(void);
void configure_tc_callbacks(void);
void tc_callback_to_toggle_pitch(
		struct tc_module *const module_inst);
void tc_callback_to_toggle_yaw(
		struct tc_module *const module_inst);
		
		
void calculate_steps_to_move(int32_t *goal_position_cpy, int32_t *current_position_cpy);
void step_yaw(bool direction);
void step_pitch(bool direction);
void display_instructions(void);
void new_serial_data(void);
void home(void);


struct usart_module usart_instance;

struct tc_module tc_instance_pitch;
struct tc_module tc_instance_yaw;

bool shoot;
volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];
uint8_t rx_buffer_cpy[MAX_RX_BUFFER_LENGTH];
uint8_t new_data_flag=0;
uint8_t *dummy;
int32_t goal_position[2]; //pan, tilt
int32_t current_position[2]; //pan, tilt
int32_t steps_to_move[2]; //pan, tilt; negative or positive to determine direction-->negative=cc, postive=c

void tc_callback_to_toggle_pitch(
	struct tc_module *const module_inst)
{
	port_pin_toggle_output_level(PIN_PB30);
}

void tc_callback_to_toggle_yaw(
	struct tc_module *const module_inst)
{
	//port_pin_toggle_output_level(LED0_PIN);
	port_pin_toggle_output_level(PIN_PB06);
}


void step_yaw(bool direction)
{
	//step yaw for 0, pitch for 1
	//implement later
	//reset delay counter in here and do checking if i should actually step--> make sure the delay has been long enough
	
	//yaw
	port_pin_set_output_level(PIN_PB07, (int)direction);
	port_pin_set_output_level(PIN_PB06, true);
	delay_us(50);
	port_pin_set_output_level(PIN_PB06, false);
	
}

void step_pitch(bool direction)
{
	//step yaw for 0, pitch for 1
	//implement later
	//reset delay counter in here and do checking if i should actually step--> make sure the delay has been long enough
	
	//yaw
	port_pin_set_output_level(PIN_PA21, (int)direction);
	port_pin_set_output_level(PIN_PA20, true);
	delay_us(50);
	port_pin_set_output_level(PIN_PA20, false);
	
}

void configure_port_pins(void)
{
	//configure outputs
	struct port_config config_port_pin;
	
	port_get_config_defaults(&config_port_pin);
	
	config_port_pin.input_pull = PORT_PIN_PULL_DOWN;
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	
	port_pin_set_config(PIN_PB06, &config_port_pin);//yaw, pin 5 EXT1
	port_pin_set_config(PIN_PB07, &config_port_pin);//yaw, pin 6 EXT1
	port_pin_set_config(PIN_PA20, &config_port_pin);//pitch, pin 5 EXT2
	port_pin_set_config(PIN_PA21, &config_port_pin);//pitch, pin 6 EXT2
	port_pin_set_config(PIN_PB30, &config_port_pin);//trigger camera
	
}

void configure_tc(void)
{
	struct tc_config config_tc;

	tc_get_config_defaults(&config_tc);

	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV16;
	config_tc.counter_8_bit.period = 5;

	tc_init(&tc_instance_pitch, TC3, &config_tc);
	
	config_tc.counter_8_bit.period = 10;
	
	tc_init(&tc_instance_yaw, TC5, &config_tc);

	tc_enable(&tc_instance_pitch);
	tc_enable(&tc_instance_yaw);

}

void configure_tc_callbacks(void)
{
	
	tc_register_callback(&tc_instance_pitch, tc_callback_to_toggle_pitch,
	TC_CALLBACK_OVERFLOW);
	tc_register_callback(&tc_instance_yaw, tc_callback_to_toggle_yaw,
	TC_CALLBACK_OVERFLOW);

	tc_enable_callback(&tc_instance_pitch, TC_CALLBACK_OVERFLOW);
	tc_enable_callback(&tc_instance_yaw, TC_CALLBACK_OVERFLOW);

}

void calculate_steps_to_move(int32_t *goal_position_cpy, int32_t *current_position_cpy)
{
	steps_to_move[0] = goal_position_cpy[0] - current_position_cpy[0];
	if(steps_to_move[0] > 800)
	{
		steps_to_move[0] = steps_to_move[0] - 1600;
	}
	if(steps_to_move[0] < -800)
	{
		steps_to_move[0] = steps_to_move[0] + 1600;
	}
	
	steps_to_move[1] = goal_position_cpy[1] - current_position_cpy[1];
	
}

void usart_read_callback(const struct usart_module *const usart_module)
{
	for(int i=0; i<MAX_RX_BUFFER_LENGTH; i++)
		rx_buffer_cpy[i]=rx_buffer[i];
	
	new_data_flag = 1;
		
	usart_write_buffer_job(&usart_instance,
		(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);

}

void usart_write_callback(const struct usart_module *const usart_module)
{
	port_pin_toggle_output_level(LED_0_PIN);
}

void configure_usart(void)
{

	struct usart_config config_usart;

	usart_get_config_defaults(&config_usart);

	config_usart.baudrate    = 57600;
	config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

	while (usart_init(&usart_instance,
			EDBG_CDC_MODULE, &config_usart) != STATUS_OK) {
	}

	usart_enable(&usart_instance);

}

void configure_usart_callbacks(void)
{
	usart_register_callback(&usart_instance,
			usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&usart_instance,
			usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);

	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);

}

void new_serial_data(void)
{
	if(new_data_flag == 1)
		{
			
			new_data_flag = 0;
			
			usart_write_buffer_job(&usart_instance, (uint8_t *)rx_buffer_cpy, MAX_RX_BUFFER_LENGTH);
			
			dummy = (uint8_t *)strtok((char *)(rx_buffer_cpy+1), ",");
			shoot = atoi((char *)dummy);
			dummy = (uint8_t *)strtok(NULL, ",");
			goal_position[0]=atoi((char *)dummy);
			dummy = (uint8_t *)strtok(NULL, ",");
			goal_position[1]=atoi((char *)dummy);
			
			calculate_steps_to_move(goal_position, current_position);
			
								
			/*
			acceleration = atoi((const char *)rx_buffer_cpy);
			max_velocity = atoi((const char *)rx_buffer_cpy+4);
			distance_to_travel = atoi((const char *)rx_buffer_cpy+8);
			usart_write_job(&usart_instance, acceleration);
			*/
			
		}
}

void home(void)
{
	current_position[0] = 0;
	current_position[1] = 0;
	//while(limitSwitch == false);
		{
			//make a step on pitch axis, not using a funcition	
		}
}


int main(void)
{
	system_init();
	configure_tc();
	configure_tc_callbacks();
	configure_port_pins();
	//delay_init();
	//configure_usart();
	//configure_usart_callbacks();
	system_interrupt_enable_global();
	
	
	//home();
	
//	uint8_t string[] = "Data Format:\r\nacceleration topspeed distancetotravel\r\n0.01rad/s^2,0.01rad/s,radians\r\n100-999,100-999,0-9\r\n";
//	usart_write_buffer_job(&usart_instance, string, sizeof(string));
	
	while (true) {
	}
	/*while (true) {

		usart_read_buffer_job(&usart_instance,
				(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);			
		new_serial_data();
		
		
		
		if(current_position[1] != goal_position[1])
		{
			if(steps_to_move[1] > 0)
			{
				step_pitch(1);
				current_position[1]++;		
			}
			if(steps_to_move[1] < 0)
			{
				step_pitch(0);
				current_position[1]--;	
			}
			delay_ms(1);
		}
		
				
		if(current_position[0] != goal_position[0])
		{
			if(steps_to_move[0] > 0)
			{
				step_yaw(1);
				current_position[0]++;		
			}
			if(steps_to_move[0] < 0)
			{
				step_yaw(0);
				current_position[0]--;	
			}
			
			if(current_position[0] < 0)
				current_position[0] = 1600 + current_position[0];
			if(current_position[0] > 1600)
				current_position[0] = current_position[0] - 1600;
				
			//delay_ms(30);
		}
				
		
		
	}
	*/
}
