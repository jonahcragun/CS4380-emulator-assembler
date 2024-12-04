    ; recursive fibonacci

    ; data
prompt .str "Please enter the Fibonacci term you would like computed: "
result1 .str "Term "
result2 .str " in the Fibonacci sequence is: "
nl .byt #10

    ; prompt user
    lda r3, prompt
    trp #5
    
    ; get input
    trp #2
    mov r0, r3
    
    ; calculate fib term
    pshr r0
    subi sp, sp, #4
    call fib
    popr r1
    popr r15 ; remove parameter from stack

    ; print results
    lda r3, result1
    trp #5
    mov r3, r0
    trp #1
    lda r3, result2
    trp #5
    mov r3, r1
    trp #1
    ldb r3, nl
    trp #3
    
    ; end prog
    trp #0

    ; fibonacci func (recursive)
fib pshr fp
    mov fp, sp

    ; push regs to stack
    pshr r0
    pshr r1
    pshr r2

    ; get param
    mov r2, fp
    addi r2, r2, #12
    ildr r2, r2

    ; calculate fib term
    
    ; put return value on stack
    mov r2, fp
    addi r2, r2, #8
    istr r0, r2

    ; put everything back
    popr r2
    popr r1
    popr r0
    popr fp

    ; ret
    ret
