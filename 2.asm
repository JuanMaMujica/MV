    mov ax,1
    mov [0],ax
    mov bx,[0]
    sub ax,1
    mov [1],ax
    mov ex,[1]
    sub ax,1
    mov [2],ax
    mov ex,[2]
    mul ax,%ffff
    mov [3],ax
    mov fx,[3]
    ldh 65535
    ldl 65535
    mov [4],ac
    shr ac,32
    mov ecx, 1
    shl ecx, 31
    shr ecx, 31    
    mov [5], ecx
    sys %F 
    stop