OPTION CASEMAP:NONE

.code

PUBLIC ASMDotProduct
PUBLIC ASMMatrixMultiply

ASMDotProduct PROC

    test r8, r8
    jz DotProductInvalid

    test rcx, rcx
    jz DotProductInvalid

    test rdx, rdx
    jz DotProductInvalid

    vxorpd ymm0, ymm0, ymm0

    cmp r8, 4
    jb DotProductRemainder

DotProductVectorLoop:

    vmovupd ymm1, YMMWORD PTR [rcx]
    vmovupd ymm2, YMMWORD PTR [rdx]

    vfmadd231pd ymm0, ymm1, ymm2

    add rcx, 32
    add rdx, 32
    sub r8, 4

    cmp r8, 4
    jae DotProductVectorLoop

DotProductRemainder:

    vextractf128 xmm1, ymm0, 1
    vaddpd xmm0, xmm0, xmm1

    vhaddpd xmm0, xmm0, xmm0

    test r8, r8
    jz DotProductReturn

DotProductScalarLoop:

    vmovsd xmm1, QWORD PTR [rcx]
    vmovsd xmm2, QWORD PTR [rdx]

    vfmadd231sd xmm0, xmm1, xmm2

    add rcx, 8
    add rdx, 8
    dec r8

    jnz DotProductScalarLoop

DotProductReturn:

    vzeroupper
    ret

DotProductInvalid:

    vxorpd xmm0, xmm0, xmm0
    vzeroupper
    ret

ASMDotProduct ENDP


ASMMatrixMultiply PROC

    mov r10, QWORD PTR [rsp+40]
    mov r11, QWORD PTR [rsp+48]

    test rcx, rcx
    jz MatrixMultiplyInvalid

    test rdx, rdx
    jz MatrixMultiplyInvalid

    test r8, r8
    jz MatrixMultiplyInvalid

    test r9, r9
    jz MatrixMultiplyInvalid

    test r10, r10
    jz MatrixMultiplyInvalid

    test r11, r11
    jz MatrixMultiplyInvalid

    sub rsp, 104

    mov QWORD PTR [rsp], rcx
    mov QWORD PTR [rsp+8], rdx
    mov QWORD PTR [rsp+16], r8
    mov QWORD PTR [rsp+24], r9
    mov QWORD PTR [rsp+32], r10
    mov QWORD PTR [rsp+40], r11

    test r9, 3
    jnz MatrixFallback

    test r11, 3
    jnz MatrixFallback

    mov QWORD PTR [rsp+48], 0

MatrixBlockRowLoop:

    mov rax, QWORD PTR [rsp+48]
    cmp rax, QWORD PTR [rsp+24]
    jae MatrixMultiplyDone

    imul rax, QWORD PTR [rsp+32]
    shl rax, 3
    add rax, QWORD PTR [rsp]
    mov QWORD PTR [rsp+72], rax

    mov rdx, QWORD PTR [rsp+32]
    shl rdx, 3

    lea rcx, [rax+rdx]
    mov QWORD PTR [rsp+80], rcx

    lea rcx, [rax+rdx*2]
    mov QWORD PTR [rsp+88], rcx

    lea rcx, [rax+rdx*2]
    add rcx, rdx
    mov QWORD PTR [rsp+96], rcx

    mov QWORD PTR [rsp+56], 0

MatrixBlockColumnLoop:

    mov rax, QWORD PTR [rsp+56]
    cmp rax, QWORD PTR [rsp+40]
    jae MatrixBlockNextRow

    vxorpd ymm0, ymm0, ymm0
    vxorpd ymm1, ymm1, ymm1
    vxorpd ymm2, ymm2, ymm2
    vxorpd ymm3, ymm3, ymm3

    mov QWORD PTR [rsp+64], 0

MatrixBlockKLoop:

    mov rax, QWORD PTR [rsp+64]
    cmp rax, QWORD PTR [rsp+32]
    jae MatrixBlockStore

    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+8]

    vmovupd ymm4, YMMWORD PTR [rax]

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+72]

    vbroadcastsd ymm5, QWORD PTR [rax]
    vfmadd231pd ymm0, ymm4, ymm5

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+80]

    vbroadcastsd ymm5, QWORD PTR [rax]
    vfmadd231pd ymm1, ymm4, ymm5

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+88]

    vbroadcastsd ymm5, QWORD PTR [rax]
    vfmadd231pd ymm2, ymm4, ymm5

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+96]

    vbroadcastsd ymm5, QWORD PTR [rax]
    vfmadd231pd ymm3, ymm4, ymm5

    inc QWORD PTR [rsp+64]
    jmp MatrixBlockKLoop

MatrixBlockStore:

    mov rax, QWORD PTR [rsp+48]
    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+16]

    mov rdx, QWORD PTR [rsp+40]
    shl rdx, 3

    vmovupd YMMWORD PTR [rax], ymm0

    add rax, rdx
    vmovupd YMMWORD PTR [rax], ymm1

    add rax, rdx
    vmovupd YMMWORD PTR [rax], ymm2

    add rax, rdx
    vmovupd YMMWORD PTR [rax], ymm3

    add QWORD PTR [rsp+56], 4
    jmp MatrixBlockColumnLoop

MatrixBlockNextRow:

    add QWORD PTR [rsp+48], 4
    jmp MatrixBlockRowLoop


MatrixFallback:

    mov QWORD PTR [rsp+48], 0

MatrixRowLoop:

    mov rax, QWORD PTR [rsp+48]
    cmp rax, QWORD PTR [rsp+24]
    jae MatrixMultiplyDone

    imul rax, QWORD PTR [rsp+32]
    shl rax, 3
    add rax, QWORD PTR [rsp]
    mov QWORD PTR [rsp+72], rax

    mov QWORD PTR [rsp+56], 0

MatrixVectorCheck:

    mov rax, QWORD PTR [rsp+40]
    cmp rax, 4
    jb MatrixScalarCheck

    sub rax, 4
    mov rdx, QWORD PTR [rsp+56]
    cmp rdx, rax
    ja MatrixScalarCheck

    vxorpd ymm0, ymm0, ymm0
    mov QWORD PTR [rsp+64], 0

MatrixVectorLoop:

    mov rax, QWORD PTR [rsp+64]
    cmp rax, QWORD PTR [rsp+32]
    jae MatrixVectorStore

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+72]
    vbroadcastsd ymm2, QWORD PTR [rax]

    mov rax, QWORD PTR [rsp+64]
    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+8]

    vmovupd ymm1, YMMWORD PTR [rax]

    vfmadd231pd ymm0, ymm1, ymm2

    inc QWORD PTR [rsp+64]
    jmp MatrixVectorLoop

MatrixVectorStore:

    mov rax, QWORD PTR [rsp+48]
    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+16]

    vmovupd YMMWORD PTR [rax], ymm0

    add QWORD PTR [rsp+56], 4
    jmp MatrixVectorCheck

MatrixScalarCheck:

    mov rax, QWORD PTR [rsp+56]
    cmp rax, QWORD PTR [rsp+40]
    jae MatrixNextRow

    vxorpd xmm0, xmm0, xmm0
    mov QWORD PTR [rsp+64], 0

MatrixScalarLoop:

    mov rax, QWORD PTR [rsp+64]
    cmp rax, QWORD PTR [rsp+32]
    jae MatrixScalarStore

    mov rax, QWORD PTR [rsp+64]
    shl rax, 3
    add rax, QWORD PTR [rsp+72]

    vmovsd xmm1, QWORD PTR [rax]

    mov rax, QWORD PTR [rsp+64]
    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+8]

    vmovsd xmm2, QWORD PTR [rax]

    vfmadd231sd xmm0, xmm1, xmm2

    inc QWORD PTR [rsp+64]
    jmp MatrixScalarLoop

MatrixScalarStore:

    mov rax, QWORD PTR [rsp+48]
    imul rax, QWORD PTR [rsp+40]
    add rax, QWORD PTR [rsp+56]
    shl rax, 3
    add rax, QWORD PTR [rsp+16]

    vmovsd QWORD PTR [rax], xmm0

    inc QWORD PTR [rsp+56]
    jmp MatrixScalarCheck

MatrixNextRow:

    inc QWORD PTR [rsp+48]
    jmp MatrixRowLoop

MatrixMultiplyDone:

    vzeroupper
    add rsp, 104
    mov eax, 1
    ret

MatrixMultiplyInvalid:

    xor eax, eax
    ret

ASMMatrixMultiply ENDP

END