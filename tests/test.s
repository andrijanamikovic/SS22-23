.section my_data
my_isr_terminal:
st %r1, my_counter
st %r4, %r12
st %r4, [%r12]
st %r7, 0xEFEF
ld %r2, %r3
ld [%r12], %r4
ld 0xFFFF, %r9
ld [%r12 + 15], %r4
ld [%r12 + my_counter], %r4
ld $3, %r2
ld 0xF3, %r2
my_counter:
end.