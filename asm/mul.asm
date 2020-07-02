                section         .text

                global          _start
                %define	 LNUM 128
_start:

                sub             rsp, 5 * LNUM * 8
                mov             rdi, rsp
                mov             rcx, LNUM
                call            read_long
                lea             rdi, [rsp + LNUM * 8]
                call            read_long
                mov             rsi, rsp
                lea             r9, [rsp + 2 * LNUM * 8]
                lea             r8, [rsp + 3 * LNUM * 8]
                call            mul_long_long
                
                
                mov             rdi, r8
                add		 rcx, rcx
                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit
                
; copies long number
;   r14 -- adress of source
;   r15 -- adress of destination
;   rcx -- length of number in qwords
copy_long_number:
                push            r14
                push            r15 
                push            rcx                
.loop:          
                mov             rax, [r14]
                mov             [r15], rax                
                add             r14,    8
                add             r15,    8
                dec             rcx
                jnz             .loop
                                
                pop             rcx
                pop             r15 
                pop             r14
                ret    
                             
; multuplies two long numbers
;    rdi --  adress of multiplyer #1 (long number)
;    rsi -- adress of multiplyer #2 (long number)
;    rcx - length of long number in qwords
; result:
;   answer is written to r8 (rcx * 2 qwords)
mul_long_long:
                push            rdi
                push            rcx
		push            r14
		push 		r15
		push		r11
		push 		rbx    
                clc
;    r10 is a qword pointer in multiplyer #2 (also an offset for adding long long)   
                xor             r10, r10
        
;    copy multiplyer #1 to r9        
                mov             r14, rdi
                mov             r15, r9
                call            copy_long_number
          
;    set_zero to r8 (result destination)
                push 		 rdi
                push		 rcx
                mov		 rdi, r8
                add		 rcx, rcx
                call		 set_zero
                pop	 	 rcx
                pop		 rdi
                
.loop:
;    add result of multiplying rdi on qword from rsi to result          
                mov             rbx, [rsi + 8 * r10]
                call            mul_long_short                
                call            add_long_long_in_mul
                inc             r10
;    restore rdi from r9              
                mov             r14, r9
                mov             r15, rdi                 
                call            copy_long_number
                cmp		 r10, rcx
                jne             .loop
                
		pop		rbx
		pop		r11
		pop		r15
		pop		r14
                pop             rcx
                pop             rdi               
                ret
                
                
                
; adds two long number with carry from mul long short
;    r8 -- address of summand #1 (long number)		
;    rdi -- address of summand #2 (long number)
;    r11 -- carry from mul (belongs to summand #2)
;    rcx -- length of long numbers in qwords
;    r10 -- offset of r8 in qwords
; result:
;    sum is written to r8
add_long_long_in_mul:
                push            r8
                push            rdi
                push            rcx
                push            r10
                clc
.loop:
                mov             rax, [rdi]
                lea             rdi, [rdi + 8]
                adc             [r8 + 8 * r10], rax
                lea             r8, [r8 + 8]
                dec             rcx
                jnz             .loop
                
                adc		 [r8 + 8 * r10], r11

                pop             r10
                pop             rcx
                pop             rdi
                pop             r8
                ret

; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
;    carry is written to r11
mul_long_short:
                push            rax
                push            rdi
                push            rcx
                xor             r11, r11
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, r11
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             r11, rdx
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg
