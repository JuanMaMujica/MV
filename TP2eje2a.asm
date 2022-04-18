;ingresa 0 y 1 , emprimir el valor decimal equivalente
		MOV	AC,0
		MOV FX,0	;inicializo variables
		
		MOV AX,%001
		MOV DX,10
		MOV CX,1
ingre:	SYS 1
		CMP [10],0
		JZ	cero; si CC es cero ingreso un cero
		CMP [10],1
		JZ	uno; si CC es cero ingreso un uno
		jmp fin ; si no es ni uno ni cero fin
		
cero:	SHL AC,1
		jmp ingre

uno:	SHL AC,1
		ADD AC,1
		jmp ingre

fin:	MOV [10],AC	; como el DX esta en 10, pongo el resultado en la memoria 10
		SYS 2
		STOP