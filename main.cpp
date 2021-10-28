/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#define threshold 250

/******Variables********/

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

/******Functions********/

void check_time(void){readtime=true;}

void state_timeout(void){change_state=true;}

void print(void){printtime=true;}

/*Function button_change:
	When the user presses the button and then releases it, 
	the boolean “mode” and the leds are switched.
	---> it is called by the second thread
*/
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

/*Function button_change:
	Prints the mode (summer or winter) and the state
	of the blinds.
*/
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

/*Function compare_values:
	Evaluate the state of the blinds in order to deliberate
	if its time to change the blinds because of the light
	In the case that is necessary it sets a timeout of 30 seconds
*/

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

/******MAIN FUNCTION********/

int main(){
		//initialization
		led_summer = 0;
		led_winter = 1;
		printf("Loading...\n");
		ti.attach_us(check_time,4000000);	//read each 4 seconds
		ti_print.attach_us(print, 2000000); //print each 2 secs
		thread.start(button_change); //starts the second thread
		
    while (1) {	
				//read the current value of the light
        if(readtime){
					light_value=input.read_u16();
					readtime=false;
					printf("Current value of light sensor: %hu\r\n", light_value);
					compare_values(light_value);
				}
				//change the state of the blinds in case that is set to true during
				//the compare_values function
				if(change_state){ 
					change_state = false;
					if(s == lowering){ s=down;}
					if(s == rising){ s=raised;}
				}
				//print the values when the 2secs timer set printtime to true
				if(printtime){
					print_values(s);
					printtime=false;
				}
		}
}


		
