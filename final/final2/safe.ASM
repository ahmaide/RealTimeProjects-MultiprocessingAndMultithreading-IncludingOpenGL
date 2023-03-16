	PROCESSOR 16F877A
	__CONFIG	0x3771

	#INCLUDE "P16F877A.INC"
	letter	EQU	30	; current letter
	CBLOCK	31 ; array to store the entered string
	first
	second
	third
	fourth
ENDC
	passwordChecker EQU 32

	; PROGRAM BEGINS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		ORG		0	
		NOP			
		goto	init

	; INTERRUPT
	ORG		004
	; here should be the check of the entered string as the intrrupts button is pressed
	
	
init	NOP
		BANKSEL 	PORTB
		CLRF 	PORTB
		BSF 	TRISB, 0

		BANKSEL TRISD
		CLRF 	TRISD
		
		BANKSEL TRISC
		MOVLW	B'00000111'	; Keypad
		MOVWF	TRISC
		BSF 	OPTION_REF, INTEDG
		BCF		STATUS, RP0
		MOVLW	0X90
		MOVFW	INTCON

main	CALL inid
		CALL initMes
		MOVLW 0x01
		movwF first
		MOVLW 0x02
		movwF second
		MOVLW 0x03
		movwF third
		MOVLW 0x01
		movwF fourth
		BANKSEL 	PORTB
		BSF 	PORTB, 2; the light pin

EnterPassword 	CALL scan
				; here the passwork must be entered


row	INCF	Key		; Count first key
	BTFSS	PORTC,0		; Check key
	GOTO	found		; and quit if on

	INCF	Key		; and repeat
	BTFSS	PORTC,1		; for second 
	GOTO	found		; key

	INCF	Key		; and repeat
	BTFSS	PORTC,2		; for third
	GOTO	found		; key
	GOTO	next		; go for next row

; Scan the keypad..........................................

scan	CLRF	Key		; Zero key count 
		BSF	3,0		; Set Carry Flag 
		BCF	PORTC,4		; Select first row
newrow	GOTO	row		; check row

next	BSF	PORTC,3		; Set fill bit
		RLF	PORTC		; Select next row
		BTFSC	3,0		; 0 into carry flag?
		GOTO	newrow		; if not, next row
		GOTO	scan		; if so, start again

found	RETURN	


initMes		BCF		Select,RS	; set display command mode
			MOVLW	0x80		; code to home cursor
			CALL	send		; output it to display
			BSF		Select,RS	; and restore data mode
			MOVLW	'E'			; load volts code
			CALL	send		; and output
			MOVLW	'n'			; load volts code
			CALL	send		; and output
			MOVLW	't'			; load volts code
			CALL	send		; and output
			MOVLW	'e'			; load volts code
			CALL	send		; and output
			MOVLW	'r'			; load volts code
			CALL	send		; and output
	
			MOVLW	' '			; load volts code
			CALL	send		; and output
	
			MOVLW	't'			; load volts code
			CALL	send		; and output
			MOVLW	'h'			; load volts code
			CALL	send		; and output
			MOVLW	'e'			; load volts code
			CALL	send		; and output
			MOVLW	'e'			; load volts code
			CALL	send		; and output
			MOVLW	' '			; load volts code
			CALL	send		; and output
			MOVLW	'c'			; load volts code
			CALL	send		; and output
			CALL	send		; and output
			MOVLW	'o'			; load volts code
			CALL	send		; and output
			CALL	send		; and output
			MOVLW	'd'			; load volts code
			CALL	send		; and output
			CALL	send		; and output
			MOVLW	'e'			; load volts code
			CALL	send		; and output
			
			BCF		Select,RS	; set display command mode
			MOVfW	pos
			CALL	send		; output it to display
			BSF		Select,RS
			
			movfw	letter
			CALL	send	
			BCF		Select,RS	; set display command mode
			MOVfW	pos
			CALL	send		; output it to display
			BSF		Select,RS
			RETURN				; done
	
	
	
			#INCLUDE "LCDIS.INC"
		
			END	;;;;;;;;;;