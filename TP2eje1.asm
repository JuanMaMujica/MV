;calcular e imprimir el promedio , de numeros naturales ingresados por teclado.
;finaliza el ingreso con un numero negativo.

;suma=0;
;cont=0;
;do{
;	scanf("%d",&n);
;	suma+=n;
;	cont++;
;}while (n>0);

;printf("%4.2f",(float)(suma/cont));
		MOV	AC,0
		MOV FX,0	;inicializo variables
		MOV AX,%001
		MOV DX,10
		MOV CX,1
ingre:	SYS 1		;ingreso numero y almaceno en [10]
		CMP [10],0	
		JN fin		; si [10] es negativo jump fin
		ADD AC,[10]	; si no es negativo Ac+=[10] y FX++
		ADD FX,1	
		JP ingre	; Vuelvo a ingresar
		
fin:	MOV [10],AC	; como el DX esta en 10, pongo el resultado en la memoria 10
		CMP FX,0
		jz	muestra
		DIV [10],FX	;divido por FX
		
muestra:	SYS 2
		STOP