section .text
global vec_assign_ones
align 64

; void scalar_assign_ones(short* x) {
;     for (int i = 0; i < 32; i += 2) {
;         x[i] = 1;
;     }
; }

vec_assign_ones:
    mov rax, 0x5555555555555555
    kmovq k1, rax
    vpbroadcastw zmm0, [rel one]
    vmovdqu16 [rdi]{k1}, zmm0
    ret

section .data
align 2
one:    dw 1

section .note.GNU-stack noalloc noexec nowrite progbits


