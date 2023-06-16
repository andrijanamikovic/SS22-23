.section my_code_main
call 0xFFFF
call 0x01
call 0xEEEE
call 0x2
jmp 0xFFFF
beq %r7, %r8, 0xEFEF
beq %r7, %r8, 5
st %r4, %r12
st %r4, 0xFFFF
.end