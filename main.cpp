/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#define threshold 250

AnalogIn input(PA_4);
DigitalIn button(PB_2);

unsigned short light_value;
bool mode = false, readtime=false, change_state=false, button_pressed=false, printtime=false, rate_state=false;;
Ticker ti, ti_print;
Timeout to;
Thread thread(osPriorityNormal, 512);
enum State {raised, down, rising, lowering};
State s;

DigitalOut led_summer(LED1);
DigitalOut led_winter(LED3);

void check_time(void){readtime=true;}
void state_timeout(void){change_state=true;}
void print(void){printtime=true;}
void button_change(void){
	while(1){
		if(button){
			if(!button_pressed){
				button_pressed = true;
				printf("Changing mode...\n");
				mode =! mode;
				led_summer =! led_summer;
				led_winter =! led_winter;
				printf("Mode: %s\n", mode ? "Summer" : "Winter");
			}
		}else{
				button_pressed = false;
		}
	}
}

void print_values(State s){
	printf("MODE: %s\n", mode ? "Summer" : "Winter");
	switch(s){
		case 0: printf("BLIND STATUS: Raised \n"); break;
		case 1: printf("BLIND STATUS: Down \n"); break;
		case 2: printf("BLIND STATUS: Rising \n"); break;
		case 3: printf("BLIND STATUS: Lowering \n"); break;
	}	
	printf("\n\n\r");
}

void compare_values(unsigned short light){
	if(mode & (light>threshold) & (s != down) & (s != lowering)){
		rate_state = true;
		s=lowering;
	}else if(!mode & (light<threshold) & (s != raised) & (s != rising)){
		rate_state = true;
		s=rising;
	}
	if(rate_state){
		rate_state = false;
		to.attach_us(state_timeout, 30000000);
	} 
}	
int main(){
		led_summer = 0;
		led_winter = 1;
		printf("Loading...\n");
		ti.attach_us(check_time,4000000);
		ti_print.attach_us(print, 2000000); 
		thread.start(button_change);
		
    while (1) {				
        if(readtime){
					light_value=input.read_u16();
					readtime=false;
					printf("Current value of light sensor: %hu\r\n", light_value);
					compare_values(light_value);
				}
				if(change_state){
					change_state = false;
					if(s == lowering){ s=down;}
					if(s == rising){ s=raised;}
				}
				if(printtime){
					print_values(s);
					printtime=false;
				}
		}
}


		
