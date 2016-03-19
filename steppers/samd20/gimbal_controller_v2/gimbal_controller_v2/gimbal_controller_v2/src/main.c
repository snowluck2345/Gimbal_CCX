/**
 * \file
 *
 * \brief SAM TC - Timer Counter Driver Quick Start
 *
 * Copyright (C) 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <asf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define STEPS_PER_REVOLUTION 1600
#define MAX_RX_BUFFER_LENGTH 16



void usart_read_callback(const struct usart_module *const usart_module);
void usart_write_callback(const struct usart_module *const usart_module);
void configure_usart(void);
void configure_usart_callbacks(void);
void configure_port_pins(void);
void configure_tc(void);
void configure_tc_callbacks(void);
void tc_callback_to_toggle_pitch(struct tc_module *const module_inst);
void tc_callback_to_toggle_yaw(struct tc_module *const module_inst);
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
uint8_t new_data_flag = 0;
uint8_t *dummy;
uint8_t serial_flag = 0;
int32_t goal_position[2]; //pan, tilt
int32_t current_position[2]; //pan, tilt
int32_t steps_to_move[2]; //pan, tilt; negative or positive to determine direction-->negative=cc, postive=c



void tc_callback_to_toggle_pitch(struct tc_module *const module_inst)
{
	//port_pin_toggle_output_level(PIN_PA20);
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
		//	delay_ms(1);
		}
}
void tc_callback_to_toggle_yaw(struct tc_module *const module_inst)
{
	//port_pin_toggle_output_level(LED0_PIN);
	//port_pin_toggle_output_level(PIN_PB06);
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
void configure_tc(void)
{
	struct tc_config config_tc;

	tc_get_config_defaults(&config_tc);

	config_tc.counter_size = TC_COUNTER_SIZE_8BIT;
	config_tc.clock_source = GCLK_GENERATOR_1;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV16;
	config_tc.counter_8_bit.period = 2;

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

	config_usart.baudrate    = 9600;
		
	config_usart.mux_setting = USART_RX_1_TX_0_XCK_1;
	config_usart.pinmux_pad0 = PINMUX_PB16C_SERCOM5_PAD0;
	config_usart.pinmux_pad1 = PINMUX_PB17C_SERCOM5_PAD1;
	config_usart.pinmux_pad2 = PINMUX_UNUSED;
	config_usart.pinmux_pad3 = PINMUX_UNUSED;

	while (usart_init(&usart_instance,
			SERCOM5, &config_usart) != STATUS_OK) {
	}

	usart_enable(&usart_instance);
	
	/*
	struct usart_config config_usart;

	usart_get_config_defaults(&config_usart);
+
	config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;

	while (usart_init(&usart_instance,
	EDBG_CDC_MODULE, &config_usart) != STATUS_OK) {
	}

	usart_enable(&usart_instance);
	*/

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
			serial_flag = 0;
			
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
	delay_init();
	configure_usart();
	configure_usart_callbacks();


	system_interrupt_enable_global();
	
	
	while (true) {

		usart_read_buffer_job(&usart_instance,
				(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);			
		new_serial_data();
		
		if(current_position[0] == goal_position[0] && current_position[1] == goal_position[1] && serial_flag == 0)
		{
			serial_flag = 1;
			uint8_t position_output[14];
			sprintf(position_output, "$%.5d,%.5dCC",(int)current_position[0], (int)current_position[1]);
			 
			usart_write_buffer_job(&usart_instance, position_output, sizeof(position_output));
				
		}
		
		if(shoot==1)
		{
			shoot = 0;
			port_pin_set_output_level(PIN_PB30, true);
			delay_ms(1000);
			port_pin_set_output_level(PIN_PB30, false);
		}
		
	}
	
}
