;
; This code acts as a NOR logic gate
; WRITTEN: 05/09/2025
; AUTHOR : Jacob JM Horstman, Sasha Dauz
;
; I/O pins
; D2: LED
; A5: pushbutton    (external pull-down)
; A4: pushbutton    (external pull-down)

; any non-repeating tasks should occur outside of the loop
SBI DDRD, 2	; configure D2 as an output pin

; loop subroutine starts here
loop:
	; start by turning the LED off
	CBI PORTD, 2
	; input data from the PINC register
	IN r16, PINC
	; mask data from the PINC register
	ANDI r16, 0x30

	; compare r16 and zero
	CPI r16, 0x00
	; if r16 = 0, turn on the LED
	BREQ turn_on_led	; if r16 is zero, the code will branch to the turn_on_led address location

	; otherwise, we can jump back to the start of the loop
	JMP loop

turn_on_led:
	SBI PORTD, 2
	; now that the LED is on, go back to the start of the loop
	JMP loop