;ingresa 0 y 1 , emprimir el valor decimal equivalente
		MOV AX,%001
		MOV DX,0
		MOV CX,1
		SYS 1
		MOV DX,1
		SYS 1
		MOV BX,[0]
		MOV CX,[1]	;fin de carga de registros

		MOV AX,0	;algoritmo
		CMP CX,0
		JNP fin
mult:	ADD AX,BX
		ADD CX,-1
		CMP CX,0
		JNZ mult

fin:  SYS 15
		stop