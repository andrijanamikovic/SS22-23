
.extern mathAdd, mathSub, mathMul, mathDiv

.global my_start

.global value0, value1, value2, value3, value4, value5, value6

.section my_code
my_start:
add r0, r1
.skip 2 # isr_error
sub r13, r12

destinations:
.word mathAdd
.word mathSub
.word mathMul
.word mathDiv