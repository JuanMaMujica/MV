;ingresa 0 y 1 , emprimir el valor decimal equivalente
		MOV AX,%001
		MOV DX,10
		MOV CX,1
		SYS 1
		
		MOV FX,-1		;empiezo en -1
		MOV [11],[10]	;guardo el valor ingresado en [11]
		MOV AX,%901		;%9 omite prompt y endline, %001 imprime decimal
		
divi:	ADD FX,1	;almacena en FX la cantidad de bits de [10]
		DIV	[10],2
		JNZ	divi	;mientras el resultado de la division no sea cero, divide por 2

		
mues:	MOV [12],1		;[12]=1
		SHL [12],FX		;[12]=1,2,4,8,16,32... dependiendo de FX
		mov [10],[11]	;[10]=valor ingresado
		AND [10],[12]	;[10]& 32 , en [10] me quedo con el bit mas significativo de [10]
		SHR [10],FX		;llevo el bit mas significativo, lo mas a la derecha posible
		ADD FX,-1		;resto FX
		sys 2			; muestro el bit
		CMP	FX,0		
		JP mues
		
		stop