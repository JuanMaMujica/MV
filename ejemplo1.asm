const EQU -5
ldh 2
ldl 64
MOV EBX,CONST
mov AH, %02
mov AL, 3
mov CH, 2
mov CL, 8
mov DH, 10
mov DL, 0 
sys %D
STOP