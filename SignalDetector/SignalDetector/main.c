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
#include <util/atomic.h>
#include <string.h>


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

typedef enum
{
	FALLING,
	RISING	
} InputCompareMode_t;

void InputCompareInit( InputCompareMode_t InputCompareMode )
{	
	INPUT_CAPTURE_DDR &= ~(1<<INPUT_CAPTURE_BP);
	INPUT_CAPTURE_PORT |= (1<<INPUT_CAPTURE_BP);

	if ( InputCompareMode == FALLING )
	{	
		TCCR1B |= ((1<<ICNC1) | (Timer1Prescaler[TIMER1_PRESCALER_1])); // Bei fallender Flanke triggern ; Prescaler einstellen
	}else if ( InputCompareMode == RISING )
	{
		TCCR1B |= ( (1<<ICNC1) | (1<<ICES1) | Timer1Prescaler[TIMER1_PRESCALER_1]); // Bei steigender Flanke triggern ; Prescaler einstellen		
	}
		
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
	InputCompareInit( FALLING );
	
	/*	Struktur initalisieren
	*/
	memset( (Pulse_t*)&Signal , 0 , sizeof(Pulse_t) );
	
    while (1) 
    {	
		if ( Signal.b0 )
		{
			if ( Signal.ulBeginn++ >= 300 )
			{
				Signal.b0 = 0;
				Signal.ulBeginn = 0;

				DEBUG_LED_PORT &= ~(1<<DEBUG_LED_BP);
					
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
			}
		}
    }
}
