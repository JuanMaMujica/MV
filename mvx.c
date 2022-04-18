#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define cantidadMR 4096


typedef struct TRegistros{
    __int32 ValorRegistro;
    char nombre[3];
} TRegistros;


typedef struct {
    char mnemonico[5];
    __int32 cod;
} elementosMnemonicos;


elementosMnemonicos mnemonicos[25];
TRegistros Registros[16];
__int32 Memoria[cantidadMR]={0};
int banderas[5] = {0};
int breakpoint = 0;

//--------------------------------------Prototipos---------------------------------------------------
void InicializaRegistros();
void cargaMnemonicos();
void leeInstruccion();
void cambiaCC(__int32 valorOp1);
__int32 leeMnemonico(__int32 instruccion,int *cantidadOperandos);
__int32 decodificaOperando(__int32 op, __int32 tipoOp);
void alamacenaRM(__int32 valorOp, __int32 tipoOp, __int32 op);
void MuestraCodigo();
void Dissasembler(int pos_memoria);


//-------------------------------------------OPERACIONES--------------------------------------------------
void mov(__int32 *a, __int32 *b){
    *a = *b;
}

void add(__int32 *a, __int32 *b){
    *a += *b;
}

void sub(__int32 *a, __int32 *b){
    *a -= *b;
}

void swap(__int32 *a, __int32 *b){
    __int32 aux = *a;
    *a = *b;
    *b = aux;
}

void mul(__int32 *a, __int32 *b){
    *a *= *b;
}

void DIV(__int32 *a, __int32 *b){
    Registros[9].ValorRegistro = *a % *b; //Guarda el resto en el AC.
    *a /= *b;
}

void cmp(__int32 *a, __int32 *b){ //Modifica el CC.
    if (*a - *b == 0)
        Registros[8].ValorRegistro = 1;
    else if (*a - *b < 0)
        Registros[8].ValorRegistro = 0x80000000;
    else
        Registros[8].ValorRegistro = 0;
}

void and(__int32 *a, __int32 *b){
    *a &= *b;
}

void or(__int32 *a, __int32 *b){
    *a |= *b;
}

void xor(__int32 *a, __int32 *b){
    *a ^= *b;
}

void shl(__int32 *a, __int32 *b){
    *a = *a << *b;
}

void shr(__int32 *a, __int32 *b){
    *a = *a >> *b;
} 

void jmp(__int32 *a){
    Registros[5].ValorRegistro = *a;
}

void jz(__int32 *a){
    if((Registros[8].ValorRegistro & 0x00000001) == 0x00000001){
        Registros[5].ValorRegistro = *a;
    }
}

void jp(__int32 *a){
    if(Registros[8].ValorRegistro == 0){ //Condición vieja: (Registros[8].ValorRegistro & 0x80000000) == 0x00000000)
        Registros[5].ValorRegistro = *a;
    }
}

void JN(__int32 *a){
    if((Registros[8].ValorRegistro & 0x80000000) == 0x80000000){
        Registros[5].ValorRegistro = *a;
    }
}

void jnz(__int32 *a){
    if((Registros[8].ValorRegistro & 0x00000001) == 0x00000000){
        Registros[5].ValorRegistro = *a;
    }
}

void jnp(__int32 *a){
    if(((Registros[8].ValorRegistro & 0x80000000) == 0x80000000) || (Registros[8].ValorRegistro == 1)){ //Si es negativo Ó 0.
        Registros[5].ValorRegistro = *a;
    }
}

void jnn(__int32 *a){
    if((Registros[8].ValorRegistro & 0x80000000) == 0x00000000){
        Registros[5].ValorRegistro = *a;
    }
}

void ldl(__int32 *a){
    *a = *a & 0x0000FFFF;
    *a += (Registros[9].ValorRegistro & 0xFFFF0000); //Le suma los 2 BYTES más significativos del AC.
    Registros[9].ValorRegistro = *a;
}

void ldh(__int32 *a){
    *a = *a << 16;
    *a += (Registros[9].ValorRegistro & 0x0000FFFF); //Le suma los 2 BYTES menos significativos del AC.
    Registros[9].ValorRegistro = *a;
}

void rnd(__int32 *a){
    Registros[9].ValorRegistro = rand() % (*a + 1);
}

void not(__int32 *a){
    *a = ~(*a);
}

void sys(__int32 *a){

    int i,j=0,x,y;
    __int32 cx=(Registros[12].ValorRegistro & 0XFFFF);
    __int32 ds=Registros[0].ValorRegistro;
    __int32 edx=Registros[13].ValorRegistro;
    __int32 ax=(Registros[10].ValorRegistro & 0XFFFF);
    char straux[30],car[4],num1[4],num2[4];

    printf("%d", *a);

    if (*a == 0X1){
        if (ax & 0x100){     //bit vale 1
            if (!(ax & 0x800)){                             // muestra prompt. si vale 1, no entra.
                printf("[%d]:\t", edx+ds+j);
            } 
            scanf("%s", straux);
            while (j<cx && j<strlen(straux)){
                Memoria[edx+ds+j]=straux[j++]; 
            }
            if(j<cx && j==strlen(straux))
                Memoria[edx+ds+j]=0X0;       
        } else{
            for(i=0x0;i<cx;i++){     // CX=0x3C  
                if (!(ax & 0x800)){                                    // muestra prompt. si vale 1, no entra.
                    printf("[%d]:\t ", edx+ds+i);
                }  
                if (ax & 0x008){
                    scanf("%x", &x);
                    Memoria[edx+ds+i]=x;
                } else if (ax & 0x004){
                    scanf("%o", &x);
                    Memoria[edx+ds+i]=x;
                } else if (ax & 0x001){
                    scanf("%d", &x);
                    Memoria[edx+ds+i]=x;
                }
            }
        }
    } else if (*a==0X2){           //sys 2
        for (i=0;i<cx;i++){
            if (!(ax & 0x800)){
                printf("[%d]:\t", edx+ds+i);     
            }
            if (ax & 0x010){ 
                if (Memoria[edx+ds+i]<=0x7E && Memoria[edx+ds+i]>=0x20)  //si es imprimible printeo como char (creo que estos son los caracteres imprimibles, pero no se)
                    printf("%c", Memoria[edx+ds+i] & 0XFF);
                else
                    printf(".");  //si no es imprimible printeo un punto
            }
            if (ax & 0x008){
                printf("%x", Memoria[edx+ds+i]);
            }
            if (ax & 0x004){
                printf("%o", Memoria[edx+ds+i]);
            }
            if (ax & 0x001){
                printf("%d", Memoria[edx+ds+i]);
            }
            if (!(ax & 0x100)){     //si el bit 8 vale 0, agrego salto de línea despues de imprimir
                printf("\n");
            } 
        }
    } else{         //sys f
        if (banderas[0]) {   //si está -b
            printf("[%d] cmd: ",Registros[5].ValorRegistro-1);
            fflush(stdin);    //muestro el ip en el prompt
            gets(car);
            if (banderas[1])
                system("cls");    //me copié de su mv vieja
            if (banderas[2])
                MuestraCodigo();
            //disassembler, nazi  //trabajare como string a lo que entre por comodidad
            if (car[0]=='p'){
                breakpoint = 1;
                    //se me ocurre una variable global que entre a sys f desde ejecucion
                    //y se setee en 1 cada vez que ponemos 'p'?
            } else if (car[0]>='0' && car[0]<='9'){          //si el primer caracter es un numero // quizá haya alguna funcion para hacer esto menos peruano xD
                breakpoint = 0;
                i=0;
                while (car[i]!='\0' && car[i]!=' '){    //concateno el numero en el array num1 hasta que haya espacio o no haya nada (ya se que no sera negativo)
                    num1[i]=car[i];
                    i++;
                }
                x=atoi(num1);         //convierto num1 a integer
                if (car[i]=='\0')    //si no hay nada, entonces no hay segundo numero
                    printf("[%d]: Hexa: %x Decimal: %d \n", x, Memoria[x], Memoria[x]);
                 else if (car[i]==' '){    //si hay espacio, hay segundo numero. no validé que entre otra cosa
                    j=0;
                    while(car[i]!='\0'){      //mientras no sea nulo, concateno en num2
                        num2[j]=car[i];
                        i++;
                    }
                    y=atoi(num2);       //convierto num2 a int
                    if (y>0 && x<y){      //si y es un entero positivo printeo entre x e y.
                        for (i=x;i<=y;i++){
                            printf("[%d]: Hexa; %x  Decimal: %d", i, Memoria[i], Memoria[i]);
                        }
                    }
                }
            } else if (car[0]=='r'){
                breakpoint = 0;
                //printf("Continuando ejecucion...");
            }
        }
    }
}

void stop(){
    Registros[5].ValorRegistro = Registros[0].ValorRegistro;
}


//-----------------------------------------MV----------------------------------------------------------

void main(int arg,char *args[]){
    
    __int32 Header[6]={0};
    FILE *archI;
    archI = fopen(args[1],"rb");

    InicializaRegistros();
    cargaMnemonicos();
    fread(Header,sizeof(__int32),6,archI);
    fread(Memoria,sizeof(__int32),Header[1],archI);
    Registros[0].ValorRegistro = Header[1];
    

    if(archI!=NULL){    
        if (arg>1){ //Si hay banderas, se fija cuáles están.
            for (int i=2; i < arg; i++){
                if (strcmp(args[i], "-b") == 0){
                    banderas[0] = 1;
                }
                if (strcmp(args[i], "-c") == 0){
                    banderas[1] = 1;
                }
                if (strcmp(args[i], "-d") == 0){
                    banderas[2] = 1;
                }
            }
        }
        if(banderas[1]){
            system("cls");
        }
        if(banderas[2]){
            MuestraCodigo();
        }
        leeInstruccion();
  

    }


}

void InicializaRegistros(){
    strcpy(Registros[0].nombre,"DS "); Registros[0].ValorRegistro=0;
    strcpy(Registros[1].nombre,"   "); Registros[1].ValorRegistro=0;
    strcpy(Registros[2].nombre,"   "); Registros[2].ValorRegistro=0;
    strcpy(Registros[3].nombre,"   "); Registros[3].ValorRegistro=0;
    strcpy(Registros[4].nombre,"   "); Registros[4].ValorRegistro=0;
    strcpy(Registros[5].nombre,"IP "); Registros[5].ValorRegistro=0;
    strcpy(Registros[6].nombre,"   "); Registros[6].ValorRegistro=0;
    strcpy(Registros[7].nombre,"   "); Registros[7].ValorRegistro=0;
    strcpy(Registros[8].nombre,"CC "); Registros[8].ValorRegistro=0;
    strcpy(Registros[9].nombre,"AC "); Registros[9].ValorRegistro=0;
    strcpy(Registros[10].nombre,"EAX"); Registros[10].ValorRegistro=0;
    strcpy(Registros[11].nombre,"EBX"); Registros[11].ValorRegistro=0;
    strcpy(Registros[12].nombre,"ECX"); Registros[12].ValorRegistro=0;
    strcpy(Registros[13].nombre,"EDX"); Registros[13].ValorRegistro=0;
    strcpy(Registros[14].nombre,"EEX"); Registros[14].ValorRegistro=0;
    strcpy(Registros[15].nombre,"EFX"); Registros[15].ValorRegistro=0;
}

void cargaMnemonicos()  //funcion que carga los mnemonicos con sus respectivos codigos en un arreglo
{
    mnemonicos[0].cod=0X0;
    strcpy(mnemonicos[0].mnemonico,"MOV");
    mnemonicos[1].cod=1;
    strcpy(mnemonicos[1].mnemonico,"ADD");
    mnemonicos[2].cod=2;
    strcpy(mnemonicos[2].mnemonico,"SUB");
    mnemonicos[3].cod=3;
    strcpy(mnemonicos[3].mnemonico,"SWAP");
    mnemonicos[4].cod=4;
    strcpy(mnemonicos[4].mnemonico,"MUL");
    mnemonicos[5].cod=5;
    strcpy(mnemonicos[5].mnemonico,"DIV");
    mnemonicos[6].cod=6;
    strcpy(mnemonicos[6].mnemonico,"CMP");
    mnemonicos[7].cod=7;
    strcpy(mnemonicos[7].mnemonico,"SHL");
    mnemonicos[8].cod=8;
    strcpy(mnemonicos[8].mnemonico,"SHR");
    mnemonicos[9].cod=9;
    strcpy(mnemonicos[9].mnemonico,"AND");
    mnemonicos[10].cod=0XA;
    strcpy(mnemonicos[10].mnemonico,"OR");
    mnemonicos[11].cod=0XB;
    strcpy(mnemonicos[11].mnemonico,"XOR");
    mnemonicos[12].cod=0XF0;
    strcpy(mnemonicos[12].mnemonico,"SYS");
    mnemonicos[13].cod=0XF1;
    strcpy(mnemonicos[13].mnemonico,"JMP");
    mnemonicos[14].cod=0XF2;
    strcpy(mnemonicos[14].mnemonico,"JZ");
    mnemonicos[15].cod=0XF3;
    strcpy(mnemonicos[15].mnemonico,"JP");
    mnemonicos[16].cod=0XF4;
    strcpy(mnemonicos[16].mnemonico,"JN");
    mnemonicos[17].cod=0XF5;
    strcpy(mnemonicos[17].mnemonico,"JNZ");
    mnemonicos[18].cod=0XF6;
    strcpy(mnemonicos[18].mnemonico,"JNP");
    mnemonicos[19].cod=0XF7;
    strcpy(mnemonicos[19].mnemonico,"JNN");
    mnemonicos[20].cod=0XF8;
    strcpy(mnemonicos[20].mnemonico,"LDL");
    mnemonicos[21].cod=0XF9;
    strcpy(mnemonicos[21].mnemonico,"LDH");
    mnemonicos[22].cod=0XFA;
    strcpy(mnemonicos[22].mnemonico,"RND");
    mnemonicos[23].cod=0XFB;
    strcpy(mnemonicos[23].mnemonico,"NOT");
    mnemonicos[24].cod=0XFF1;
    strcpy(mnemonicos[24].mnemonico,"STOP");
}

void leeInstruccion(){
    __int32 sysB=0XF;
    int cantidadOperandos=0;
    __int32 instruccion,mnemonico,tipoOp1,tipoOp2,op1,op2,valorOp1,valorOp2,sectorOp2;
    void (*fun[])(__int32 *, __int32 *) = {mov, add, sub,swap,mul,DIV,cmp,shl,shr,and,or,xor};
    void (*fun2[])(__int32 *) = {sys,jmp, jz,jp,JN,jnz,jnp,jnn,ldl,ldh,rnd,not};

    while (Registros[5].ValorRegistro>=0 && Registros[5].ValorRegistro<Registros[0].ValorRegistro){
       // printf("%d\n",Registros[0].ValorRegistro);
        instruccion = Memoria[Registros[5].ValorRegistro];
        Registros[5].ValorRegistro++;
        
        mnemonico = leeMnemonico(instruccion,&cantidadOperandos);
        if(cantidadOperandos == 2){
            tipoOp1 = (instruccion >> 26) & 0X3;
            tipoOp2 = (instruccion >> 24) & 0X3;
            op1 = (instruccion >> 12) & 0XFFF;
            op2 = instruccion & 0XFFF;
            valorOp1 = decodificaOperando(op1,tipoOp1);
            valorOp2 = decodificaOperando(op2,tipoOp2);
            sectorOp2 = op2>>4 & 0X3;
            if(tipoOp2 == 0){
                valorOp2=valorOp2<<20;  //propaga el bit de signo de un operando inmediato negativo, de ser positivo no importa el corrimiento queda igual
                valorOp2=valorOp2>>20;  
            } else if(tipoOp2 == 1) {
                switch (sectorOp2){
                case 1:
                    valorOp2 = valorOp2<<24;
                    valorOp2 = valorOp2>>24;
                    break;
                case 2:
                    valorOp2 = valorOp2<<24;
                    valorOp2 = valorOp2>>24;
                    break;
                case 3:
                    valorOp2 = valorOp2<<16;
                    valorOp2 = valorOp2>>16;
                default:
                    break;
                }
            }
            (*fun[mnemonico])(&valorOp1,&valorOp2); //llama a la instruccion correspondiente dependiendo del mnemonico
            if(mnemonico!=0X6){ // alamcena los valores calculados anteriormente en los registros o memoria correspondiente menos en el cmp 
               // printf("%d %08X\n", Registros[5].ValorRegistro,op1);
               // printf("%d %08X\n", Registros[5].ValorRegistro,op2);
                alamacenaRM(valorOp1,tipoOp1,op1);
                alamacenaRM(valorOp2,tipoOp2,op2);
            //    printf("%d %08X %d\n",Registros[5].ValorRegistro ,Memoria[op1+Registros[0].ValorRegistro],op1);
            }
            //printf("%d %08X\n",Registros[5].ValorRegistro ,Memoria[op1+Registros[0].ValorRegistro]); 
            if(mnemonico != 0X0 && mnemonico != 0X3 && mnemonico !=0X6 ){ // cambia el valor de CC seguun el resultado que se calcule
                cambiaCC(valorOp1);
            }
            

        } else if(cantidadOperandos == 1){
            tipoOp1 = (instruccion>>22) & 0X3;
            op1 = instruccion & 0XFFFF; 
            valorOp1 = decodificaOperando(op1,tipoOp1);
            //printf("%X: %08X  %08X\n",mnemonico,Memoria[op1+Registros[0].ValorRegistro],op1);
            (*fun2[((mnemonico>>24)&0XF)])(&valorOp1);
           // printf("%X: %08X  %08X\n",mnemonico,Memoria[op1+Registros[0].ValorRegistro],op1);
            if (mnemonico==0XFA || mnemonico==0XFB){   // RND, NOT
                alamacenaRM(valorOp1,tipoOp1,op1);
            }
            if (mnemonico==0XFB){
                cambiaCC(valorOp1);
            }
        } else {
            stop();
        }
        if(breakpoint == 1 && mnemonico != mnemonicos[12].cod){
            sys(&sysB);
        }
    }  

}

void cambiaCC(__int32 valorOp1){
   
   if(valorOp1 == 0){
        Registros[8].ValorRegistro=0X1;
    } else if(valorOp1 < 0){
        Registros[8].ValorRegistro=0X80000000;
    } else {
        Registros[8].ValorRegistro=0X0;
    }
}

__int32 leeMnemonico(__int32 instruccion,int *cantidadOperandos){
    __int32 mnemonico = instruccion>>20 & 0XFFF;

    if((mnemonico & 0XF00) != 0XF00){
        mnemonico = mnemonico>>8;
        *cantidadOperandos = 2;        
    } else if((mnemonico & 0XFF0) != 0XFF0){
        mnemonico = mnemonico>>4;
        *cantidadOperandos = 1;
    } else {
        *cantidadOperandos = 0;
    }
    return mnemonico;
}

__int32 decodificaOperando(__int32 op, __int32 tipoOp){
    __int32 sectorRegistro;
    __int32 registro;
    __int32 valorOp = op;
    if(tipoOp == 1){//es registro 
        sectorRegistro = (op>>4)&0X3;
        registro = op & 0XF;
        if(sectorRegistro == 0){
            valorOp = Registros[registro].ValorRegistro;
        } else if(sectorRegistro==1){
            valorOp = Registros[registro].ValorRegistro & 0XFF;
        } else if(sectorRegistro==2){
            valorOp =(Registros[registro].ValorRegistro & 0XFF00)>>8;
        } else if(sectorRegistro==3){
            valorOp = Registros[registro].ValorRegistro & 0XFFFF;
        }
    } else if(tipoOp == 2) {    //es directo
        valorOp = Memoria[op+Registros[0].ValorRegistro];
    }
    return valorOp; // si es inmediato lo devuelve igual
}

void alamacenaRM(__int32 valorOp, __int32 tipoOp, __int32 op){
    __int32 sectorRegistro;
    __int32 registro;
    if(tipoOp == 1){        // de registro
        sectorRegistro = (op>>4)&0X3;
        registro = op & 0XF;
        if(sectorRegistro == 0){
            Registros[registro].ValorRegistro = valorOp;
        } else if(sectorRegistro==1){
            Registros[registro].ValorRegistro= (Registros[registro].ValorRegistro & 0XFFFFFF00) | (valorOp & 0XFF);
        } else if(sectorRegistro==2){
            Registros[registro].ValorRegistro = (Registros[registro].ValorRegistro & 0XFFFF00FF) | ((valorOp<<8) & 0XFF00);
        } else if(sectorRegistro==3){
            Registros[registro].ValorRegistro = (Registros[registro].ValorRegistro & 0XFFFF0000) | (valorOp & 0XFFFF);
        }
      
    } else if(tipoOp == 2){ //de directo
        Memoria[op+Registros[0].ValorRegistro] = valorOp;
    }
}

void MuestraCodigo(){
    printf("Codigo:");
    for (int i = 0; i < Registros[0].ValorRegistro; i++)
        Dissasembler(i);
    //MUESTRA DE REGISTROS
    printf("\nRegistros:\n");
    for(int j = 0; j < 16; j++){
        printf("%3s = %12d |", Registros[j].nombre, Registros[j].ValorRegistro);
        if (j % 4 == 3)
            printf("\n");
    }
}

void Dissasembler(int pos_memoria){
    __int32 inst=Memoria[pos_memoria],op,sectorReg,nroMnemonico;

    if((pos_memoria == Registros[5].ValorRegistro) && Registros[5].ValorRegistro != 0)
        printf("\n>");
    else
        printf("\n ");
    printf("[%04d]: %02X %02X %02X %02X  %3d: ",pos_memoria,(inst>>24)&0x000000FF,(inst>>16)&0x000000FF,(inst>>8)&0x000000FF,(inst&0x000000FF),pos_memoria+1);

    if((inst & 0xFF000000) == 0xFF000000){ //Si es de 0 operandos
        nroMnemonico = (inst>>20)&0x00F;
        printf("%s\t\t",mnemonicos[nroMnemonico + 23].mnemonico); //Mnemónico
    }
    else if((inst & 0xF0000000) == 0xF0000000){ //Si es de un operando
        nroMnemonico= ((inst&0x0F000000)>>24);
        printf("%s\t\t",mnemonicos[nroMnemonico +12].mnemonico); //Mnemónico
        if((inst & 0x00C00000)==0x00800000){ //Directo
            printf("[%d]",inst&0x0000FFFF);
        }
        else if((inst & 0x00C00000)==0x00400000){ //De Registro
            op = inst & 0XFFFF;
            if((op & 0XF) < 10 ){
                printf("%s", Registros[op & 0XF].nombre);
            } else {
                sectorReg = (op>>4) & 0X3;
                switch(sectorReg){
                    case 0:
                        printf("%s",Registros[op & 0XF].nombre);
                        break;
                    case 1:
                        printf("%XL",op & 0XF);
                        break;
                    case 2:
                        printf("%XH",op & 0XF);
                        break;
                    case 3:
                        printf("%XX",op & 0XF);
                        break;
                }
            }
        }
        else { //Supongo inmediato
            printf("%4d",inst&0x0000FFFF);
        }
    }
    else{ //Si es de dos operandos
        nroMnemonico= ((inst&0xF0000000)>>28) & 0x0000000F; //Mnemónico
        printf("%s\t\t",mnemonicos[nroMnemonico].mnemonico);

        if((inst & 0x0C000000)==0x08000000) //Si operando 1 es directo
            printf("[%d], ",((inst&0x00FFF000)>>12));
        else if((inst & 0x0C000000)==0x04000000){ //Si operando 1 es de registro
            op = (inst>>12) & 0XFFF;
            if((op & 0XF) < 10 ){
                printf("%s", Registros[op & 0XF].nombre);
            } else {
                sectorReg = (op>>4) & 0X3;
                switch(sectorReg){
                    case 0:
                        printf("%s, ",Registros[op & 0XF].nombre);
                        break;
                    case 1:
                        printf("%XL, ",op & 0XF);
                        break;
                    case 2:
                        printf("%XH, ",op & 0XF);
                        break;
                    case 3:
                        printf("%XX, ",op & 0XF);
                        break;
                }
            }
        }
        else { // Si operando 1 es inmediato
            op=(inst&0x00FFF000)>>12;
            op=op<<20;
            op=op>>20;
            printf("%4d, ",op);
        }
        if((inst & 0x03000000)==0x02000000) //Si operando 2 es directo
            printf("[%d]",(inst&0x00000FFF));

        else if((inst & 0x03000000)==0x01000000){ //Si operando 2 es de registro
            op = inst & 0XFFF;
            if((op & 0XF) < 10 ){
                printf("%s", Registros[op & 0XF].nombre);
            } else {
                sectorReg = (op>>4) & 0X3;
                switch(sectorReg){
                    case 0:
                        printf("%s",Registros[op & 0XF].nombre);
                        break;
                    case 1:
                        printf("%XL",op & 0XF);
                        break;
                    case 2:
                        printf("%XH",op & 0XF);
                        break;
                    case 3:
                        printf("%XX",op & 0XF);
                        break;
                }
            }
        }
        else { // Si operando 2 es inmediato
            op=(inst&0x00000FFF);
            op=op<<20;
            op=op>>20;
            printf("%d",op);
        }
    }
}
