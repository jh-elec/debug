/*
 * taster_entprellen.c
 *
 * Created: 28.03.2019 07:41:03
 * Author : Hm
 */ 

#include <avr/io.h>
#include "Switch.h"

#define CHANGE( X ) (*(uint8_t*)(0x3A) = X)

int main(void)
{
	Switch_t Taster;

	SwitchInit( &PORTD , 0x01 , &Taster );
	
    while (1) 
    {
		SwitchRead( &Taster , &PORTD );	
		
		if ( Taster.Info & 1<<0 )
		{
			Taster.Info = 0;
			PORTB ^= 1<<0;
		}
    }
}

