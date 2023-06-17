.section my_data
.extern my_code
my_isr_terminal:
st %r4, value2
st %r4, adssdad
st %r7, fdsfdsfs
st %r10, value1
ld %r2, %r3
ld [%r12], %r4
ld 0xFFFF, %r9
ld [%r12 + 15], %r4
ld $3, %r2
ld my_code, %r2
value1:
.end