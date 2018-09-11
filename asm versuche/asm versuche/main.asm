;
; asm versuche.asm
;
; Created: 09.09.2018 10:33:59
; Author : Jan Homann
;
#include <m32def.inc>

.dseg
	msg:
	.db "Hello World!" , 0 , 0 

.cseg

	.def tmp8 = r16
	
	.equ F_CPU			= 16000000	; Systemfrequenz
	.equ F_DIV			= 1		; Systemteiler
	
	.equ LED_DDR_PORT	= DDRD	; Led DatenRichtungsRegister
	.equ LED_PORT		= PORTD	; Led Port
	.equ LED_bp			= 4		; Led Bit Position




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

				ldi tmp8 , 1<<LED_bp	; DatenRichtungsRegister Bit setzen
				out LED_DDR_PORT , tmp8	; Wert an DDRx übergeben

main:
				
				rcall ledOn ; Welcher Sprung ist dafür jetzt besser geeignet?
				rcall delay100ms
				rcall ledOff
				rcall delay100ms

				rjmp main

/*	Led einschalten
*/
ledOn:		
				cbi LED_PORT , LED_bp
				ret

/*	Led ausschalten
*/
ledOff:			sbi LED_PORT , LED_bp
				ret




/*	Verzögerungsschleife(n)
*/
.equ var10ms	= 16000
.def delayHigh	= r24
.def delayLow	= r25



delay10ms:
				ldi delayHigh , high( var10ms )
				ldi delayLow  , low( var10ms )
				rcall delayLoop
				ret
delay100ms:
				ldi tmp8 , 100
delay100msLoop :
				rcall delay10ms
				dec tmp8
				brne delay100msLoop
				ret

/*	Hier werden hauptsächlich die Taktzyklen verbraten..
*/
delayLoop:
				sbiw delayHigh : delayLow , 1
				brne delayLoop
				ret
				
