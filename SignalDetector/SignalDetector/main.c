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
#define DETECTING_EDGE			RISING

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
	uint32_t	Beginn;
	
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
	/*	+++Register: TCCR1B+++
	+	• Bit 7 – ICNC1: Input Capture Noise Canceler
	+	Setting this bit (to one) activates the Input Capture Noise Canceler. When the Noise Canceler is
	+	activated, the input from the Input Capture Pin (ICP1) is filtered. The filter function requires four
	+	successive equal valued samples of the ICP1 pin for changing its output. The Input Capture is
	+	therefore delayed by four Oscillator cycles when the Noise Canceler is enabled.
	+	
	+	• Bit 6 – ICES1: Input Capture Edge Select
	+	This bit selects which edge on the Input Capture Pin (ICP1) that is used to trigger a capture
	+	event. When the ICES1 bit is written to zero, a falling (negative) edge is used as trigger, and
	+	when the ICES1 bit is written to one, a rising (positive) edge will trigger the capture.
	+	When a capture is triggered according to the ICES1 setting, the counter value is copied into the
	+	Input Capture Register (ICR1). The event will also set the Input Capture Flag (ICF1), and this
	+	can be used to cause an Input Capture Interrupt, if this interrupt is enabled.
	+	When the ICR1 is used as TOP value (see description of the WGM13:0 bits located in the
	+	TCCR1A and the TCCR1B Register), the ICP1 is disconnected and consequently the Input Capture
	+	function is disabled.
	*/	
	
	#if ( DETECTING_EDGE == RISING )
	TCCR1B |= ((1<<ICES1) | Timer1Prescaler[TIMER1_PRESCALER_1]); // Bei steigender Flanke triggern ; Prescaler einstellen
	#elif ( DETECTING_EDGE == FALLING )
	TCCR1B |= Timer1Prescaler[TIMER1_PRESCALER_1]; // Bei fallender Flanke triggern ; Prescaler einstellen
	#endif
	
	
	/* +++Register: TIMSK+++
	+	• Bit 5 – TICIE1: Timer/Counter1, Input Capture Interrupt Enable
	+	When this bit is written to one, and the I-flag in the Status Register is set (interrupts globally
	+	enabled), the Timer/Counter1 Input Capture Interrupt is enabled. The corresponding Interrupt
	+	Vector (See “Interrupts” on page 44.) is executed when the ICF1 Flag, located in TIFR, is set.
	*/
	TIMSK1 |= (1<<ICIE1); // Input Capture Interrupt aktivieren
	
	sei(); // Interrupts aktivieren	
}


ISR(TIMER1_CAPT_vect)
{
	Signal.b0 = 1; // Es kommt ein neues Signal rein.. Zähler fängt in der main an zu zählen
	Signal.Beginn = 0;
	Signal.uiIndex++;
	
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
			if ( Signal.Beginn++ >= 30e3 )
			{
				Signal.b0 = 0;
				Signal.Beginn = 0;
				
				uint8_t Edges = Signal.uiIndex; // Aktuelle Anzahl an Flanken merken
							
				if ( Edges == 11 )// Signal -> Anlage sperren
				{
					SIGNAL_SPERREN_PORT |= (1<<SIGNAL_SPERREN_BP);
					_delay_ms(500);
					SIGNAL_SPERREN_PORT &= ~(1<<SIGNAL_SPERREN_BP);
					_delay_ms(500);
					Signal.uiIndex = 0;
				} else if	( Edges == 14 )// Signal -> Annahme
				{
					SIGNAL_ANNAHME_PORT |= (1<<SIGNAL_ANNAHME_BP);
					_delay_ms(500);
					SIGNAL_ANNAHME_PORT &= ~(1<<SIGNAL_ANNAHME_BP);
					_delay_ms(500);
					Signal.uiIndex = 0;
				} else if	( Edges >= 15 && Edges <= 17 )// Signal -> Klingel
				{
					SIGNAL_KLINGEL_PORT |= (1<<SIGNAL_KLINGEL_BP);
					_delay_ms(500);
					SIGNAL_KLINGEL_PORT &= ~(1<<SIGNAL_KLINGEL_BP);
					_delay_ms(500);
					Signal.uiIndex = 0;
				}
				else
				{
					Signal.uiIndex = 0;
					for ( uint8_t i = 0 ; i < 10 ; i++ )
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
			}
		}
    }
}
