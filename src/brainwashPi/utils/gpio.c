#include <stdio.h>
#include <unistd.h>
#include <bcm2835.h>

#define PIN RPI_GPIO_P1_13

int pins [17] = {0,1,4,7,8,9,10,11,14,15,17,18,21,22,23,24,27};


int main () {
	int i;
	if(!bcm2835_init())
		return 1;
		uint8_t test;
	while(1) {

		for (i=0;i<17;i++) {
			if (bcm2835_gpio_lev(pins[i]) == HIGH) {
			    printf("H ");
			} else {
				printf("L ");
			}
		}
		printf("\n");
	}
	bcm2835_close();
	return 0;
}
