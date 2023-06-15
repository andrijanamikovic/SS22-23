.section my_code_main
call 0xFFFF
beq %r1, %r2, 0x27
bne %r1, %r2, 0x35
bne %r1, %r2, 0xFFFF
.section my_data
my_isr_terminal:
st %r1, my_counter
st %r4, %r12
st %r4, [%r12]
ld %r2, %r3
ld [%r12], %r4
ld [%r12 + 15], %r4
ld [%r12 + my_counter], %r4
ld $3, %r2
ld 0xF3, %r2

.end