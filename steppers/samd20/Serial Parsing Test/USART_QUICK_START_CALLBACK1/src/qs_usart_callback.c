
#include <asf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void usart_read_callback(const struct usart_module *const usart_module);
void usart_write_callback(const struct usart_module *const usart_module);

void configure_usart(void);
void configure_usart_callbacks(void);

void display_instructions(void);

struct usart_module usart_instance;

#define MAX_RX_BUFFER_LENGTH 16

volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];
uint8_t rx_buffer_cpy[MAX_RX_BUFFER_LENGTH];
uint8_t new_data_flag=0;

uint8_t acceleration;
uint8_t max_velocity;
uint8_t distance_to_travel;

uint32_t goal_position[2]; //pan, tilt
bool shoot;

uint8_t *dummy;

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


int main(void)
{
	system_init();

	configure_usart();
	configure_usart_callbacks();

	system_interrupt_enable_global();

	//uint8_t string[] = "Data Format:\r\nacceleration topspeed distancetotravel\r\n0.01rad/s^2,0.01rad/s,radians\r\n100-999,100-999,0-9\r\n";
	//usart_write_buffer_job(&usart_instance, string, sizeof(string));
	
	
	

	while (true) {

		usart_read_buffer_job(&usart_instance,
				(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
				
				
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
			
			
			
			
			/*
			acceleration = atoi((const char *)rx_buffer_cpy);
			max_velocity = atoi((const char *)rx_buffer_cpy+4);
			distance_to_travel = atoi((const char *)rx_buffer_cpy+8);
			usart_write_job(&usart_instance, acceleration);
			*/
			
		}
	}

}
