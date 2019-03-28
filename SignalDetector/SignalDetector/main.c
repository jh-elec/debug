/*
 * SignalDetector.c
 *
 * Created: 23.03.2019 19:28:22
 * Author : Jan
 */ 

#define F_CPU      16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>

#define TIMER_1_PRESCALER		
#define TICKS_VAL (F_CPU/256)


#define MAX_PULSES_OF_SIGNALS	10
#define MAX_EDGES_OF_SIGNAL		( MAX_PULSES_OF_SIGNALS * 2 )
#define DETECTING_EDGE			FALLING

#define SIGNAL_ANNAHME_DDR		DDRB
#define SIGNAL_ANNAHME_PORT		PORTB
#define SIGNAL_ANNAHME_BP		4

#define SIGNAL_SPERREN_DDR		DDRB
#define SIGNAL_SPERREN_PORT		PORTB
#define SIGNAL_SPERREN_BP		3

#define SIGNAL_KLINGEL_DDR		DDRB
#define SIGNAL_KLINGEL_PORT		PORTB	
#define SIGNAL_KLINGEL_BP		2


#define INPUT_CAPTURE_DDR		DDRB
#define INPUT_CAPTURE_PORT		PORTB
#define INPUT_CAPTURE_BP		0

#define _DEBUG_

#ifdef	_DEBUG_
#define DEBUG_LED_DDR			DDRB
#define DEBUG_LED_PORT			PORTB
#define DEBUG_LED_BP			5
#endif 


typedef enum
{
	TIMER1_PRESCALER_1,
	TIMER1_PRESCALER_8,
	TIMER1_PRESCALER_64,
	TIMER1_PRESCALER_256,
	TIMER1_PRESCALER_1024,
	
	__TIMER1_MAX__	
	
}Timer1Prescaler_e;

uint8_t Timer1Prescaler[__TIMER1_MAX__] =
{
	(1<<CS10),
	(1<<CS11),
	((1<<CS11) | (1<<CS10)),
	(1<<CS12),
	((1<<CS12) | (1<<CS10))
};

typedef struct  
{
	uint8_t		uwLength[MAX_EDGES_OF_SIGNAL]; // steigende + fallende Flanke(n)
	uint8_t		uiIndex;
	uint32_t	ulBeginn;
	uint32_t	ulTimeout;
	
	union
	{
		uint8_t uiState;
		uint8_t b0 :1;
		uint8_t b1 :1;
		uint8_t b2 :1;
		uint8_t b3 :1;
		uint8_t b4 :1;
		uint8_t b5 :1;
		uint8_t b6 :1;
		uint8_t b7 :1;
	};
}Pulse_t;
volatile Pulse_t Signal;

void InputCompareInit( void )
{	
	INPUT_CAPTURE_DDR &= ~(1<<INPUT_CAPTURE_BP);
	INPUT_CAPTURE_PORT |= (1<<INPUT_CAPTURE_BP);
	
	#if ( DETECTING_EDGE == RISING )
	TCCR1B |= ((1<<ICES1) | Timer1Prescaler[TIMER1_PRESCALER_1]); // Bei steigender Flanke triggern ; Prescaler einstellen
	#elif ( DETECTING_EDGE == FALLING )
	TCCR1B |= Timer1Prescaler[TIMER1_PRESCALER_1]; // Bei fallender Flanke triggern ; Prescaler einstellen
	#endif
	
	TIMSK1 |= (1<<ICIE1); // Input Capture Interrupt aktivieren
	
	sei(); // Interrupts aktivieren	
}

ISR(TIMER1_CAPT_vect)
{
	Signal.b0 = 1;
	Signal.uiIndex++;
	
	Signal.ulBeginn = 0;
	Signal.ulTimeout = 0;
	
	#ifdef _DEBUG_
	DEBUG_LED_PORT |= (1<<DEBUG_LED_BP);
	#endif
}


int main(void)
{
	DEBUG_LED_DDR		|= (1<<DEBUG_LED_BP);
	SIGNAL_ANNAHME_DDR	|= (1<<SIGNAL_ANNAHME_BP);
	SIGNAL_KLINGEL_DDR	|= (1<<SIGNAL_KLINGEL_BP);
	SIGNAL_SPERREN_DDR	|= (1<<SIGNAL_SPERREN_BP);
		
	/*	Input Compare für Flankenzählung konfigurieren
	*/
	InputCompareInit();
	
	/*	Struktur initalisieren
	*/
	memset( (Pulse_t*)&Signal , 0 , sizeof(Pulse_t) );
	
	for ( uint8_t i = 0 ; i < 10 ; i++ )
	{
		SIGNAL_SPERREN_PORT |= (1<<SIGNAL_SPERREN_BP);
		SIGNAL_ANNAHME_PORT |= (1<<SIGNAL_ANNAHME_BP);
		SIGNAL_KLINGEL_PORT |= (1<<SIGNAL_KLINGEL_BP);
		_delay_ms(25);
		SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
		SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
		SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
		_delay_ms(100);
	}
	
    while (1) 
    {	
		if ( Signal.b0 )
		{
			Signal.b3 = 0; // Timeout Flag löschen...
			if ( Signal.ulBeginn++ >= 300 )
			{
				Signal.b0 = 0;
				Signal.ulBeginn = 0;
				
				Signal.uiIndex; // Aktuelle Anzahl an Flanken merken
							
				if ( Signal.uiIndex == 11 )// Signal -> Anlage sperren
				{
					SIGNAL_SPERREN_PORT |= (1<<SIGNAL_SPERREN_BP);
					_delay_ms(50);
					SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
					_delay_ms(50);
					Signal.uiIndex = 0;
				} else if	( Signal.uiIndex == 14 )// Signal -> Annahme
				{
					SIGNAL_ANNAHME_PORT |= (1<<SIGNAL_ANNAHME_BP);
					_delay_ms(50);
					SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
					_delay_ms(50);
					Signal.uiIndex = 0;
				} else if	( Signal.uiIndex >= 15 )// Signal -> Klingel
				{
					SIGNAL_KLINGEL_PORT |= (1<<SIGNAL_KLINGEL_BP);
					_delay_ms(50);
					SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
					_delay_ms(50);
					Signal.uiIndex = 0;
				}
				else
				{
					Signal.uiIndex = 0;
					for ( uint8_t i = 0 ; i < 3 ; i++ )
					{
						DEBUG_LED_PORT &= ~(1<<DEBUG_LED_BP);
						SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
						SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
						SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
						_delay_ms(100);
						DEBUG_LED_PORT |= (1<<DEBUG_LED_BP);
						SIGNAL_SPERREN_PORT |= (1<<SIGNAL_SPERREN_BP);
						SIGNAL_ANNAHME_PORT |= (1<<SIGNAL_ANNAHME_BP);
						SIGNAL_KLINGEL_PORT |= (1<<SIGNAL_KLINGEL_BP);					
						_delay_ms(25);
					}
						DEBUG_LED_PORT &= ~(1<<DEBUG_LED_BP);
						SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
						SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
						SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
				}
				DEBUG_LED_PORT &= ~(1<<DEBUG_LED_BP);
			}
		}else
		{
			static uint32_t ulStandbyBlink = 0;
			if ( !Signal.b3 ) // Signal Timeout bzw. lange kein Signal erkannt ( Signal.b3 == 1 )
			{
				if ( Signal.ulTimeout++ > 40e5 )
				{
					Signal.b3 = 1; // Timeout bzw. Standby		
				}	
			}else
			{
				if ( ulStandbyBlink < 20e5 )
				{
					SIGNAL_SPERREN_PORT |= (1<<SIGNAL_SPERREN_BP);
					SIGNAL_ANNAHME_PORT |= (1<<SIGNAL_ANNAHME_BP);
					SIGNAL_KLINGEL_PORT |= (1<<SIGNAL_KLINGEL_BP);
				}else if ( ulStandbyBlink > 20e5 && ulStandbyBlink < 40e5 )
				{
					SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
					SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
					SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
				}else
				{
					ulStandbyBlink = 0;
				}
			}
		}
    }
}
