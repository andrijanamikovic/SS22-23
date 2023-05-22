
.extern mathAdd, mathSub, mathMul, mathDiv

.global my_start

.global value0, value1, value2, value3, value4, value5, value6

.section my_code
my_start:
add %r2, %r1
.skip 2 # isr_error
sub %r13, %r12
mul pc, %r7
beq %r1, %r2, destinations
csrwr %r1, %handler
csrrd %status, %r3

destinations:
.word mathAdd
.word mathSub
.word mathMul
.word mathDiv