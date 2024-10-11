val1 .int #10
val2 .int #20

; code section

    jmp main
main ldr r0, val1
    ldr r1, val2
    add r0, r0, r1
	mov r3, r0
    trp #3
