	PROCESSOR 16F877A
	__CONFIG	0x3771

	#INCLUDE "P16F877A.INC"
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

letter	EQU	30	; current letter
pos		EQU	31	; value for currsor position
sp		EQU	32	; variable to store space ascii
ms		EQU	33	; time counter

CBLOCK	34 ; array to store the entered string
	one
	two
	three
	four
	five
ENDC

readEnd	EQU	39
calc1	EQU 3B
calc2	EQU 3C

; PROGRAM BEGINS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		ORG		0	
		NOP			
		goto	init
	
; Interrupt ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		ORG		004
		INCF	ms
		BCF		INTCON, 2
		BTFSC	readEnd, 0
		GOTO	EOI
		MOVFW	ms
		SUBLW	0x20
		BNZ		EOI
		MOVFW 	letter
		SUBLW	' '
		BZ		skip
		MOVFW 	letter
		MOVWF	INDF
		INCF	FSR, F

		INCF	pos
skip	CALL 	term
		CLRF 	ms
		MOVLW	0x0C
		MOVWF	TMR0
		MOVLW	'A'
		MOVWF	letter
		BCF		Select,RS	; set display command mode
		MOVfW	pos
		CALL	send		; output it to display
		BSF		Select,RS
EOI		RETFIE

; Port & display setup.....................................

init		nop
	
			BANKSEL PORTB
			BSF 	TRISB, 1
	
			BANKSEL	TRISD		; Select bank 1
	    	CLRF	TRISD          	
	
			BANKSEL OPTION_REG
			MOVLW	0x47		; TMR0 initialisation code
			MOVWF	OPTION_REG	
	
			BANKSEL PORTD
			CLRF	TMR0
			MOVLW	b'10100000'	; INTCON init. code
			MOVWF	INTCON

; Initial Values ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
initVar		CALL	inid		; Initialise the display

			MOVLW 	'A'
			MOVWF	letter
	
			MOVLW 	0
			MOVWF	ms
	
			MOVLW 	0xC0
			MOVWF	pos
			
			MOVLW 	B'01011011'
			MOVWF 	sp
	
			MOVLW 	0
			MOVWF	one
			MOVLW 	0
			MOVWF	two
			MOVLW 	0
			MOVWF	three
			MOVLW 	0
			MOVWF	four
			MOVLW 	0
			MOVWF	five
			MOVLW 	0
			MOVWF	readEnd
			MOVLW 	0x34
			MOVWF 	FSR
	
			CALL	initMes	
			
			CALL 	nextChar
			GOTO	display
start		GOTO	start		; loop to main loop forever


nextChar	BTFSS	PORTB, 1
			GOTO 	pressed
			GOTO 	cont	
pressed		INCF	letter
			CLRF	ms
			MOVFW	letter
			SUBLW	0x5B
			BTFSS 	STATUS,Z
			goto 	noZero
			GOTO	zero
zero		MOVLW	' '
			MOVWF	letter
			GOTO	cont
 
noZero		MOVFW	letter
			SUBLW	'!'
			BTFSS 	STATUS,Z
			GOTO 	cont
			GOTO	spaceCh
spaceCh		MOVLW	'A'
			MOVWF	letter

cont		BTFSC	readEnd, 0
			GOTO	endString
			MOVFW	letter
			CALL 	send
			BCF		Select,RS	; set display command mode
			MOVfW	pos
			CALL	send		; output it to display
			BSF		Select,RS
			MOVLW	D'255'
			CALL 	xms
			GOTO	nextChar
endString	RETURN	



term		MOVFW	letter
			SUBLW	' '
			BZ		dis	
			MOVFW	pos
			SUBLW	0xC5
			BNZ		retur
dis			BSF		readEnd, 0
			GOTO 	EOI	
retur		RETURN


display		CALL 	inidOff

disLoop		MOVLW   0x38
			MOVWF   calc1
			MOVLW   0x80
			MOVWF   calc2

loop9		BCF		Select,RS	; set display command mode
			MOVLW	0x80		
			CALL	send		; output it to display
			BSF		Select,RS
			BTFSS	PORTB, 1
			GOTO	initVar
			CALL	printWord
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			call 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			call 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			DECF	calc1
			MOVFW 	calc1
			SUBLW	33
			BNZ		loop9
			
			MOVLW   0x34
			MOVWF   calc1

loop8		BCF		Select,RS	; set display command mode
			MOVFW	calc2		
			CALL	send		; output it to display
			BSF		Select,RS	; and restore data mode
			MOVLW	' '			; load volts code
			CALL	send		; and output
			BCF		Select,RS	; set display command mode
			INCF 	calc2
			MOVFW	calc2		
			CALL	send		; output it to display
			BSF		Select,RS
			BTFSS	PORTB, 1
			GOTO	initVar
			CALL	printWord
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			MOVFW 	calc2
			SUBLW	90
			BNZ		loop8

			MOVLW   0xD0
			MOVWF   calc2

loop7		DECF	calc2
			BCF		Select,RS	; set display command mode
			MOVFW	calc2		
			CALL	send		; output it to display
			BSF		Select,RS	; and restore data mode
			CALL	printWord
			BTFSS	PORTB, 1
			GOTO	initVar
			BSF		Select,RS	; and restore data mode
			MOVLW	' '			; load volts code
			CALL	send
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVFW 	calc2
			SUBLW	0xC0
			BNZ		loop7

loop6		BCF		Select,RS	; set display command mode
			MOVLW	0xC0		; code to home cursor
			CALL	send		; output it to display
			BSF		Select,RS	; and restore data mode
			CALL	printWord
			BTFSS	PORTB, 1
			GOTO	initVar
			BSF		Select,RS	; and restore data mode
			MOVLW	' '			; load volts code
			CALL	send
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			INCF	calc1
			MOVFW 	calc1
			SUBLW	0x39
			BNZ		loop6

			BCF		Select,RS	; set display command mode
			MOVLW	0xC0		; code to home cursor
			CALL	send		; output it to display
			BSF		Select,RS	; and restore data mode
			MOVLW	' '			; load volts code
			CALL	send
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			MOVLW	D'250'
			CALL 	xms
			BTFSS	PORTB, 1
			GOTO	initVar
			GOTO	disLoop
			GOTO	start


printWord	MOVFW 	calc1
			MOVWF 	FSR
loop		BTFSS	PORTB, 1
			GOTO	initVar
			MOVFW	INDF
			SUBLW	0
			BTFSC	STATUS, Z
			RETURN
			MOVFW	INDF
			CALL	send
			INCF 	FSR, F
			
			MOVFW	FSR
			SUBLW	39		
			BNZ		loop
			RETURN


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
	
			MOVLW	'S'			; load volts code
			CALL	send		; and output
			MOVLW	't'			; load volts code
			CALL	send		; and output
			MOVLW	'r'			; load volts code
			CALL	send		; and output
			MOVLW	'i'			; load volts code
			CALL	send		; and output
			MOVLW	'n'			; load volts code
			CALL	send		; and output
			MOVLW	'g'			; load volts code
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
		
			END	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;