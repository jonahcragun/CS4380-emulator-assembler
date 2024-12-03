 ; program a: fibonacci

 ; data
prompt .str "Please enter the Fibonacci term you would like computed: "
result1 .str "Term "
result2 .str " in the Fibonacci sequence is: "
nl .byt #10
term .bts #4

 ; prompt user
 lda r3, prompt
 trp #5

 ; get input
 trp #2
 str r3, term

 ; calculate fib term
 movi r0, #0
 movi r1, #1

fib subi r3, r3, #1
    mov r2, r1
    add r1, r0, r1
    mov r0, r2
    bnz r3, fib 

 ; print results
 lda r3, result1
 trp #5
 ldr r3, term
 trp #1
 lda r3, result2
 trp #5
 mov r3, r0
 trp #1
 ldb r3, nl
 trp #3

 ; end prog
 trp #0
