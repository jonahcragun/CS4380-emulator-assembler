    ; Program D: Prime Number Generator

    ; data
pt1 .str "Welcome to the Prime Number Generator."
pt2 .str "This program searches for and displays the first 20 prime numbers greater than or equal to a user provided lower bound."
pt3 .str "Please enter a lower bound: "
rs1 .str "The first 20 prime numbers greater than or equal to "
rs2 .str " are:"
nl  .byt #10
nums .bts #80
val .int #0

    ; promt user for value
    lda r3, pt1
    trp #5
    ldb r3, nl
    trp #3
    trp #3
    lda r3, pt2
    trp #5
    ldb r3, nl
    trp #3
    trp #3
    lda r3, pt3
    trp #5

    ; get user value
    trp #2
    mov r0, r3
    str r0, val

    ; calculate and store prime numbers
    movi r2, #0
    lda r5, nums
lp1 pshr r0
    subi sp, sp, #1
    call is_prime
    popb r1
    popr r15 ; get param off stack

    ; add num to mem if prime
    subi r4, r1, #1
    bnz r4, elp
    istr r0, r5
    addi r2, r2, #1
    addi r5, r5, #4

elp addi r0, r0, #1
    subi r4, r2, #20
    bnz r4, lp1
    

    ; print results
    lda r3, rs1
    trp #5
    ldr r3, val
    trp #1
    lda r3, rs2
    trp #5
    ldb r3, nl
    trp #3
    lda r5, nums
pnt ildr r3, r5
    trp #1
    ldb r3, nl
    trp #3
    subi r2, r2, #1
    addi r5, r5, #4
    bnz r2, pnt

    ; exit program
    trp #0

    ; *****************************************************
    ; is_prime func (return 1 if number is prime, 0 if not)
    ; *****************************************************
is_prime pshr fp
    mov fp, sp

    ; store reg val on stack
    pshr r0
    pshr r1
    pshr r2
    pshr r4

    ; get param from stack
    mov r2, fp
    addi r2, fp, #9
    ildr r0, r2

    ; not prime if <= 1
    subi r1, r0, #1
    blt r1, np
    brz r1, np

    ; prime if == 2
    subi r1, r0, #2
    brz r1, ip

    ; determine if num is prime
    divi r2, r0, #2
    movi r1, #2
plp pshr r1
    pshr r0
    subi sp, sp, #4
    call mod
    popr r4
    addi sp, sp, #8

    ; check if remainder is 0
    brz r4, np

    ; get ready for next iteration
    addi r1, r1, #1
    sub r4, r2, r1
    bgt r4, plp

    ; is prime
ip  movi r1, #1
    jmp p_end

    ; not prim
np  movi r1, #0
    jmp p_end

    ; put ret val on stack
p_end mov r2, fp
    addi r2, r2, #8
    istb r1, r2

    ; put everythin back
    popr r4
    popr r2
    popr r1
    popr r0
    popr fp

    ; ret
    ret

    ; ********************
    ; modulus function
    ; ********************
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
