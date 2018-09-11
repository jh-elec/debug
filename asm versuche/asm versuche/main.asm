;
; asm versuche.asm
;
; Created: 09.09.2018 10:33:59
; Author : Jan Homann
;

	.def tmp8 = r16
	
	.equ F_CPU			= 16000000	; Systemfrequenz
	.equ F_DIV			= 1		; Systemteiler
	
	.equ LED_DDR_PORT	= DDRD	; Led DatenRichtungsRegister
	.equ LED_PORT		= PORTD	; Led Port
	.equ LED_bp			= 4		; Led Bit Position


.macro ldi_hl 
  ldi @0h,high(@1)
  ldi @0l,low(@1)
.endmacro

	ldi_hl r15,r16

initHardware:
				/* StackPointer
				*
				*	Der Stackpointer wird am Ende des RAM´s angelegt.
				*	Von dort aus wächst er je nach Bedarf nach "hinten" raus.
				*
				*	Einige Controller initalisieren den Stack schon automatisch..
				*	Der ATmega32 ist noch keiner davon...
				*	Wichtig! Der Stack muss vor aufruf eines Unterprogrammes oder 
				*	einen eingehenden / auftretenden Interrupt initalisiert werden.
				*
				*/
				ldi tmp8 , high(ramend) ; Stackpointer initalisieren ( high Byte von der Adresse )
				out sph , tmp8 ; Addresse übergeben

				ldi tmp8 , low(ramend) ; Stackpointer initalisieren ( low Byte von der Adresse )
				out spl , tmp8 ; Addresse übergeben

				//ldi tmp8 , 1<<LED_bp	; DatenRichtungsRegister Bit setzen
				//out LED_DDR_PORT , tmp8	; Wert an DDRx übergeben
				sbi  LED_DDR_PORT , LED_bp ; einzelnes Bit setzen

				
				rcall ledOff ; Welcher Sprung ist dafür jetzt besser geeignet?
				rcall delay10ms
				rcall ledOn
				rcall delay10ms

/*	Led ausschalten
*/
ledOff:		
				cbi LED_PORT , LED_bp
				ret

/*	Led einschalten
*/
ledOn:			sbi LED_PORT , LED_bp
				ret




/*	Verzögerungsschleife(n)
*/
#define CALC_DELAY_CYCLES( _ms_ )	  ( ( F_CPU / F_DIV ) / _ms_ )	
.def delayHigh	= r24
.def delayLow	= r25

delay1ms:
				ldi delayHigh , high( CALC_DELAY_CYCLES( 1 ) )
				ldi delayLow , low( CALC_DELAY_CYCLES( 1 ) )
				rcall delayLoop
				ret

delay10ms:
				ldi delayHigh , high( CALC_DELAY_CYCLES( 10 ) )
				ldi delayLow , low( CALC_DELAY_CYCLES( 10 ) )
				rcall delayLoop
				ret

delay100ms:
				ldi delayHigh , high( CALC_DELAY_CYCLES( 100 ) )
				ldi delayLow , low( CALC_DELAY_CYCLES( 100 ) )
				rcall delayLoop
				ret


/*	Hier werden hauptsächlich die Taktzyklen verbraten..
*/
delayLoop:
				dec delayHigh
				dec delayLow
				brne delayLoop
				ret
				
