	.file	"main.c"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.global	spi_transfer
	.type	spi_transfer, @function
spi_transfer:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	out 0xf,r24
	sbi 0xe,6
	ldi r24,lo8(27)
.L2:
	sbic 0xe,6
	rjmp .L5
	out 0xd,r24
	rjmp .L2
.L5:
	in r24,0xf
	ret
	.size	spi_transfer, .-spi_transfer
.global	setup
	.type	setup, @function
setup:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	sbi 0x17,1
	sbi 0x18,1
	cbi 0x1a,6
	sbi 0x1b,6
	sbi 0x1a,5
	sbi 0x1a,4
	sbi 0x1a,3
	sbi 0x1a,2
	cbi 0x1b,3
	ldi r24,lo8(38)
	rcall spi_transfer
	ldi r24,lo8(6)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(37)
	rcall spi_transfer
	ldi r24,lo8(64)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(36)
	rcall spi_transfer
	ldi r24,lo8(3)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(48)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(42)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	ldi r24,lo8(-77)
	rcall spi_transfer
	sbi 0x1b,3
	ret
	.size	setup, .-setup
.global	loop
	.type	loop, @function
loop:
	push r28
	push r29
/* prologue: function */
/* frame size = 0 */
/* stack size = 2 */
.L__stack_usage = 2
	cbi 0x1b,3
	ldi r24,lo8(39)
	rcall spi_transfer
	ldi r24,lo8(16)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(-31)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(-1)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(16)
	rcall spi_transfer
	ldi r28,lo8(addr)
	ldi r29,hi8(addr)
.L8:
	ldi r24,lo8(-1)
	rcall spi_transfer
	st Y+,r24
	ldi r18,hi8(addr+5)
	cpi r28,lo8(addr+5)
	cpc r29,r18
	brne .L8
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(32)
	rcall spi_transfer
	ldi r24,lo8(10)
	rcall spi_transfer
	sbi 0x1b,3
	cbi 0x1b,3
	ldi r24,lo8(-96)
	rcall spi_transfer
	ldi r28,lo8(password)
	ldi r29,hi8(password)
.L9:
	ld r24,Y+
	rcall spi_transfer
	ldi r24,hi8(password+6)
	cpi r28,lo8(password+6)
	cpc r29,r24
	brne .L9
	sbi 0x1b,3
	sbi 0x1b,2
	ldi r25,lo8(40)
1:	dec r25
	brne 1b
	cbi 0x1b,2
	ldi r18,lo8(1599999)
	ldi r24,hi8(1599999)
	ldi r25,hlo8(1599999)
1:	subi r18,1
	sbci r24,0
	sbci r25,0
	brne 1b
	rjmp .
	nop
	in r25,0x17
	ldi r24,lo8(2)
	eor r24,r25
	out 0x17,r24
/* epilogue start */
	pop r29
	pop r28
	ret
	.size	loop, .-loop
	.section	.text.startup,"ax",@progbits
.global	main
	.type	main, @function
main:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	rcall setup
.L13:
	rcall loop
	rjmp .L13
	.size	main, .-main
.global	password
	.section	.rodata
	.type	password, @object
	.size	password, 6
password:
	.ascii	"carlos"
	.comm	addr,5,1
	.ident	"GCC: (GNU) 4.9.2"
.global __do_copy_data
.global __do_clear_bss
