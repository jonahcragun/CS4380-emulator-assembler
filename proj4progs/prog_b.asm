    ; modulus function

    ; data
p1  .str "Please enter an integer dividend: "
p2  .str "Please enter an integer divisor: "
r1  .str " divided by "
r2  .str " results in a remainder of: "
nl  .byt #10

    ; prompt dividend
    lda r3, p1
    trp #5

    ; get int
    trp #2
    mov r0, r3

    ; prompt divisor
    lda r3, p2
    trp #5

    ; get int
    trp #2
    mov r1, r3

    ; calculate modululs
    pshr r1
    pshr r0
    subi sp, sp, #4 ; allocate room for return value
    call mod
    popr r2
    popr r4
    popr r4

    ; print results
    mov r3, r0
    trp #1
    lda r3, r1
    trp #5
    mov r3, r1
    trp #1
    lda r3, r2
    trp #5
    mov r3, r2
    trp #1
    ldb r3, nl
    trp #3

    ; end program
    trp #0

    ; modulus function
mod pshr fp 

    ; set cur fram pointer
    mov fp, sp

    ; store reg values on stack
    pshr r0
    pshr r1
    pshr r2
    pshr r4

    ; get param 1
    mov r2, fp
    addi r2, fp, #12
    ildr r0, r2

    ; get param 2
    addi r2, r2, #4
    ildr r1, r2

    ; determine if divisor is + or -
    movi r4, #1
    bgt r0, neg
    muli r0, r0, #-1
    movi r4, #-1

neg bgt r1, mlp
    muli r1, r1, #-1

    ; perform mod operation
mlp sub r0, r0, r1
    bgt r0, mlp
    brz r0, end
    add r0, r0, r1
    mul r0, r0, r4

    ; store result on stack
end mov r2, fp
    addi r2, r2, #8
    istr r0, r2

    ; put things back to how they were
    popr r4
    popr r2
    popr r1
    popr r0
    popr fp

    ; ret
    ret
