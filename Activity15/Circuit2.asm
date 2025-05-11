; This code acts as a NOR or XNOR logic gate
; If a toggle switch is asserted, the code acts as a XNOR
; If deasserted, the code acts as a NOR
; WRITTEN: 05/09/2025
; AUTHOR : Jacob JM Horstman
; I/O pins
; D3: Toggle Switch (external pull-down -> Necessary to clean noise/mechanical switch bounce?)
; D2: LED
; A5: pushbutton    (external pull-down)
; A4: pushbutton    (external pull-down)

; any non-repeating tasks should occur outside of the loop
SBI DDRD, 2	; configure D2 as an output pin

; loop subroutine starts here
loop:
	; start by turning the LED off
	CBI PORTD, 2
	; read toggle
	; input data from the PIND register
	IN r20, PIND
	; mask data from the PIND register
	ANDI r20, 0x08
	; compare to 0x08 (if it is equal to 0x08, then the toggle is set)
	CPI r20, 0x08
	; if r20 = 0x08, XNOR logic
	BREQ XNOR

	; otherwise NOR logic
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

XNOR:
	; input data from the PINC register
	IN r16, PINC
	IN r17, PINC
	; mask data from the PINC register
	ANDI r16, 0x20
	ANDI r17, 0x10
	LSR r16
	; compare r16 and r17
	CP r16, r17
	; if r16 != r17, turn on the LED
	BREQ turn_on_led	; if r16 != r17, the code will branch to the turn_on_led address location
	; otherwise, we can jump back to the start of the loop
	JMP loop

turn_on_led:
	SBI PORTD, 2
	; now that the LED is on, go back to the start of the loop
	JMP loop
