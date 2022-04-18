;ingresa 0 y 1 , emprimir el valor decimal equivalente
		MOV AX,%001
		MOV DX,0
		MOV CX,1
		SYS 1
		MOV BX,[0];fin de carga de registros	/BX=n
		MOV CX,BX
		ADD CX,-1		;CX=n-1
		MOV DX,BX
		ADD DX,-1
		
		SYS 15

		MOV AX,0	;algoritmo
BxC:	CMP CX,0
		JNP fin
mult:	ADD AX,BX
		ADD CX,-1
		CMP CX,0
		JNZ mult
		MOV BX,DX
		ADD DX,-1
		MOV CX,DX
		CMP DX,1
		JNZ BxC
		
fin: 	CMP AX,0
		JNZ fiin
		ADD AX,1

fiin: SYS 15
		stop
