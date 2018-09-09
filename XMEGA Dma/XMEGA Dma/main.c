/*
 * XMEGA Dma.c
 *
 * Created: 30.08.2018 06:42:37
 * Author : Hm
 */ 

#define DMA_SET_TEST_CHANNEL		0


#include "C:\Users\Hm\Documents\Atmel Studio\7.0\C-Librarys\trunk\XMEGA Dma\xmega_dma.h"
#include "C:\Users\Hm\Documents\Atmel Studio\7.0\C-Librarys\trunk\XMEGA Dma\xmega_dma.c"
#include "C:\Users\Hm\Documents\Atmel Studio\7.0\C-Librarys\trunk\UART\XMEGA\xmega_usart.h"
#include "C:\Users\Hm\Documents\Atmel Studio\7.0\C-Librarys\trunk\UART\XMEGA\xmega_usart.c"

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include "xmega_init.h"


char *dmaTxBuff = "1234\r\n";

char dmaRxBuff[100] = "";

uint8_t dmaConvertString ( void )
{
	char* a = "Hallo Welt!12345";
	char* b = "----------------";


	dmaEnableController();
	dmaSetChannel		( DMA_SET_TEST_CHANNEL );
	dmaSetBlockSize		( DMA_SET_TEST_CHANNEL , 16 );
	dmaSetSrcChDir		( DMA_SET_TEST_CHANNEL , a + 15 , SrcDirectionDec , SrcReloadBlock );
	dmaSetDestChDir		( DMA_SET_TEST_CHANNEL , b , DestDirectionInc , DestReloadBlock );
	dmaSetRepeatCount	( DMA_SET_TEST_CHANNEL , 1 );
	dmaSetBurstLength	( DMA_SET_TEST_CHANNEL , BurstLength1Byte );
	dmaSetSingleShot	( DMA_SET_TEST_CHANNEL , false );
	dmaSetTriggerSource	( DMA_SET_TEST_CHANNEL , DMA_CH_TRIGSRC_OFF_gc );
	dmaEnableChannel	( DMA_SET_TEST_CHANNEL );
	dmaTrigger			( DMA_SET_TEST_CHANNEL );
	
	/* Hier könnten wir noch andere Dinge im Hauptprogramm tun, während der String
	 * im Hintergrund invertiert wird. 16 Bytes gehen allerdings sehr schnell, weswegen
	 * hier nicht viel Spielraum für weitere Berechnungen bleibt.
	 */

	// Wir warten auf das Ende der kompletten Transaktion
	while ( ! dmaIsTransactionComplete( DMA_SET_TEST_CHANNEL ) );	

	return 0;
}

uint8_t	dmaUsartInit( void )
{
	dmaEnableController();
	dmaSetChannel		( 0 );
	dmaSetBlockSize		( 0 , strlen( dmaTxBuff) );
	dmaSetSrcChDir		( 0 , dmaTxBuff , SrcDirectionInc , SrcReloadBlock );
	dmaSetDestChDir		( 0 , (uint32_t*)&USARTE0.DATA , DestDirectionFixed , DestReloadBlock );
	dmaSetRepeatCount	( 0 , 0 );	
	dmaSetBurstLength	( 0 , BurstLength1Byte );
	dmaSetSingleShot	( 0 , false );
	dmaSetTriggerSource	( 0 , DMA_CH_TRIGSRC_USARTE0_DRE_gc );
	dmaEnableChannel	( 0 );
	dmaTrigger			( 0 );	
	
	return 0;
}



int main(void)
{
	xmega32MhzInit();
	
	usartInit		( &USARTE0 , 9600 );
		
	_delay_ms(100);
	
	dmaCopyAsychron( DMA_SET_TEST_CHANNEL , dmaTxBuff , dmaRxBuff , 4 );

	usartPutStr( "Rx: " );
	usartPutStr( dmaRxBuff );
	usartPutStr( "\r\n" );

    while (1)
    {

    }
}

