;
; asm versuche.asm
;
; Created: 09.09.2018 10:33:59
; Author : Jan
;
#include <m32def.inc>

/*	Beginn des Datensegments
*/
/*****************************************************************/
.dseg
	msg:
	.db "Hello World!" , 0 , 0 
/*****************************************************************/


/*	Beginn des Kodesegments
*/
/*****************************************************************/
.cseg

	.def tmp8 = r16
	
	.equ F_CPU			= 16000000	; Systemfrequenz
	.equ F_DIV			= 1			; Systemteiler
	
	.equ LED_DDR_PORT	= DDRD	; Led DatenRichtungsRegister
	.equ LED_PORT		= PORTD	; Led Port
	.equ LED_bp			= 4		; Led Bit Position
/*****************************************************************/



initHardware:
				/*	StackPointer
				*
				*	Der Stackpointer wird am Ende des RAM´s angelegt.
				*	Von dort aus wächst er je nach Bedarf nach "hinten"
				*	( also Richtung Anfang des RAM´s ) raus.
				*
				*	Einige Controller initalisieren den Stack schon automatisch richtig.
				*	Andere Initalisieren ihn auf Anfang vom RAM Bereich.
				*
				*	Der ATmega32 ist noch keiner davon, der ihn am Ende des RAM´s
				*	initalisiert.
				*	Das heißt das müssen wir als erstes tun.
				*
				*	Wichtig! Der Stack muss vor aufruf eines Unterprogrammes oder 
				*	einen eingehenden / auftretenden Interrupt initalisiert werden.
				*
				*/
				ldi tmp8 , high(ramend) ; Stackpointer initalisieren ; 1 Takt
				out sph , tmp8 ; Addresse übergeben ; 1 Takt

				ldi tmp8 , low(ramend) ; Stackpointer initalisieren ; 1 Takt
				out spl , tmp8 ; Addresse übergeben ; 1 Takt

				ldi tmp8 , 1<<LED_bp	; DatenRichtungsRegister Bit setzen ; 1 Takt
				out LED_DDR_PORT , tmp8	; Wert an DDRx übergeben ; 1 Takt

				sei ; Interrupts freischalten

main:			
				rcall ledOn ;3 Takte
				rcall delay100ms ; 3 Takte
				rcall ledOff ; 3 Takte
				rcall delay100ms ; 3 Takte

				rjmp main ; 2 Takte


/*	Led einschalten
*/
/***********************************************/
ledOn:		
				cbi LED_PORT , LED_bp ; 2 Takte
				ret ; 4 Takte
/***********************************************/


/*	Led ausschalten
*/
/***********************************************/
ledOff:			
				sbi LED_PORT , LED_bp ; 2 Takte
				ret ; 4 Takte
/***********************************************/



/*	Verzögerungsschleifen
*
*	Konfigurationen für die Verzögerungsschleife(n)
*	var1ms = ( ( F_CPU / F_DIV ) / 1000000 )
*/
.equ preload1ms		= ( ( F_CPU / F_DIV ) / 1000000 )
.def delayHigh		= r24
.def delayLow		= r25

/*	Verzögerungsschleife 1 Millisekunde
*/
/***********************************************/
delay1ms:
				ldi delayHigh , high( preload1ms ) ; 1 Takt
				ldi delayLow  , low( preload1ms ) ; 1 Takt
delay1msLoop:			
				sbiw delayHigh : delayLow , 1	; Register -1 ; 2 Takte
				brne delay1msLoop ; Solange "delay1msLoop" aufrufen bis delayHigh : delayLow == 0 ;
				ret ; 4 Takte
/***********************************************/

/*	Verzögerungsschleife 10 Millisekunden
*/
/***********************************************/
delay10ms:		
				ldi tmp8 , 10 ; 1 Takt
delay10msLoop:
				
				rcall delay1ms ; 4 Takte
				dec tmp8 ; 1 Takt
				brne delay10msLoop ; 1 / 2 Takte
				ret ; 4 Takte
/***********************************************/

/*	Verzögerungsschleife 100 Millisekunden
*/
/***********************************************/
delay100ms:
				ldi tmp8 , 100 ; 1 Takt
delay100msLoop :
				rcall delay1ms ; 4 Takte
				dec tmp8 ; 1 Takt
				brne delay100msLoop ; 1 / 2 Takte
				ret ; 4 Takte
/***********************************************/
