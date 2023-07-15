.global value2
.section my_code
my_start:
    ld $0xFFFFFEFE, %sp

    ld $1, %r1
    push %r1
    ld $1, %r1
    push %r1
    call 0xF0000000
    st %r1, value2

    ld $2, %r1
    push %r1
    ld $1, %r1
    push %r1
value2:
.end