/*
 * SoftI2cTest.c
 *
 * Created: 03.04.2019 17:12:00
 * Author : Jan Homann
 */ 

#include "i2csoft.h"
#include <avr/io.h>

int main(void)
{
	I2cSoftInit();
		
    while (1) 
    {
 		I2cSoftStart( );
 		I2cSoftWrite( 0xA2 ); // Slave Adresse
		I2cSoftWrite( 13 );
 		I2cSoftWrite( 0x83 );
 		I2cSoftStop( );
    }
}

