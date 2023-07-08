.global value1
.section my_data
  ld $1, %r1
  push %r1
    call value1
    ld $1, %r1
    push %r1
.section my_code
value1:
.end