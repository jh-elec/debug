/*
 * SoftI2cTest.c
 *
 * Created: 03.04.2019 17:12:00
 * Author : Jan Homann
 */ 

#include "i2csoft.h"
#include <avr/io.h>

tI2c SoftI2c;

int main(void)
{
	SoftI2c.Port = &PORTC;
	SoftI2c.Scl = 1<<PC0;
	SoftI2c.Sda = 1<<PC1;
	I2cSoftInit( &SoftI2c );
	
	
	uint8_t Commando[] = { 13 , 0x83 }; // Frequenz Modus + 1Hz

	
	
    while (1) 
    {
 		*SoftI2c.Port |=  (1<<SoftI2c.Scl);
		*SoftI2c.Port |=  (1<<SoftI2c.Sda);
		 
 		*SoftI2c.Port &= ~(1<<SoftI2c.Scl);
		*SoftI2c.Port &= ~(1<<SoftI2c.Sda);
		
// 		I2cSoftStart( &SoftI2c );
// 		I2cSoftWrite( &SoftI2c , 0xA2 ); // Slave Adresse
// 		I2cSoftWrite( &SoftI2c , 13 );
// 		I2cSoftWrite( &SoftI2c , 0x83 );
// 		I2cSoftStop( &SoftI2c );
    }
}

