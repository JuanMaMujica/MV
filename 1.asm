ldh 0 ; Código del ES
ldl 64 ; offset del ES
mov EBX, AC ; Dirección del buffer de lectura
mov AH, %03 ; Leer del disco
mov AL, 9 ;
mov CH, 2 ; Cilindro 2
mov CL, 8 ; Cabeza 8
mov DH, 3 ; Sector 10
mov DL, 0 ; Unidad de disco 0
sys %D ; Indica que se realiza la lectura
STOP