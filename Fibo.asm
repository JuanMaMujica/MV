CONST1 EQU ELPEPE
CONST2 EQU 4
CONST3 EQU ABC

		sys %F
		mov eax,23
		mov		[10],0		;inicializo variables
		mov 	[11],1
otro: 	cmp 	[11],100	;compara
		jp 		fin			;salta si llego a 100 o mas
		swap 	[10],[11]
		add 	[11],[EAX]
		mov		ax, %001
		mov		cx,1
		mov		edx,10
		sys		2
		jmp		otro
fin:	stop