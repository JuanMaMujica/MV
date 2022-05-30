#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define cantidadMR 8192
#define HD 512  //header de discos
typedef unsigned char BYTE;


typedef struct TRegistros{
    __int32 ValorRegistro;
    char nombre[3];
} TRegistros;


typedef struct {
    char mnemonico[5];
    __int32 cod;
} elementosMnemonicos;

typedef struct{
    __int32 tipoArchivo;
    __int32 numVersion;
    char GUID[32];
    __int32 fechaCreacion;
    __int32 horaCreacion;
    BYTE tipo;
    BYTE cantCilindros;
    BYTE cantCabezas;
    BYTE cantSectores;
    __int32 tamSector;
    char relleno[211];
} sectorDisco;

typedef struct nodo{
    char nombreDisco[30];
    short int numDisco;
    struct nodo *sig;
} nodo;

typedef struct nodo *TListaDiscos;

typedef struct{
    __int32 tipoArchivo;
    __int32 numVersion;
    unsigned char GUID[16];
    __int32 fechaCreacion;
    __int32 horaCreacion;
    BYTE tipo;
    BYTE cantCilindros;
    BYTE cantCabezas;
    BYTE cantSectores;
    __int32 tamSector;
    char relleno[211];
} sectorDisco;

elementosMnemonicos mnemonicos[32];
TRegistros Registros[16];
__int32 Memoria[cantidadMR]={0};
int banderas[5] = {0};
int breakpoint = 0;
TListaDiscos LD=NULL;

//--------------------------------------Prototipos---------------------------------------------------
__int32 Header[6]={0};
__int8 Errores[5]={0}; //Puede haber 5 errores en la ejecucion
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
void slen(__int32 *a, __int32 *b){
   __int8 res=0;
   __int32 aux=*b;
   while (Memoria[aux]!='\0')
   {
       res++;
       aux++;
   }
  // printf("La longitud es de %d \n", res);
   *a=res;
}

void smov(__int32 *a, __int32 *b){
    __int32 pos1=*a;
    __int32 pos2=*b;
    /*
    printf("Entrando al SMOV \n");
    printf("Memoria[%d]=%d \n",pos1,Memoria[pos1]);
    printf("Memoria[%d]=%d \n",pos2,Memoria[pos2]);
    */
    while((char)Memoria[pos2]!='\0'){
        //printf("Moviendo el caracter %c a la posicion %d \n", Memoria[pos2], pos1);
        Memoria[pos1]=Memoria[pos2];
        //printf("Memoria[%d]: %c \n",pos1, Memoria[pos1]);
        pos1++;
        pos2++;
    }
   // printf("Memoria[%d]: %c", *a, Memoria[*a]);
   Memoria[pos1]='\0';
}

void scmp(__int32 *a, __int32 *b){         
    __int32 pos1=*a;
    __int32 pos2=*b;
    __int8 resta=0;
    while(Memoria[pos1]!=0 && Memoria[pos2]!=0 && resta==0){        //si no tenemos en cuenta el primer caracter ANDA BIEN. probar con memoria[pos1+1] && memoria[pos2+1] en la condicion
        printf("Restare %c - %c \n", Memoria[pos1], Memoria[pos2]);
        resta=Memoria[pos1]-Memoria[pos2];
        pos1++;
        pos2++;
    }

    if (resta<0){
            //printf("a<b \n");
            Registros[8].ValorRegistro= 0x80000000;
             }
    else if (resta>0){
            Registros[8].ValorRegistro=0x0;
            //printf("a>b \n");
             }
    else    {
        //printf("a=b \n");
        Registros[8].ValorRegistro=0X1;

    }
}

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

__int16 estado= 0x2; // codigo 2= no se hicieron operaciones. la hago global por si debo consultar con el %00??? 

void sys(__int32 *a){

    int i,j=0,x,y;
    __int16 cx=(Registros[12].ValorRegistro) & 0xFFFF;
    __int32 ds=Registros[0].ValorRegistro & 0xFFFF;
    __int32 edx=Registros[13].ValorRegistro & 0xFFFF;
    __int16 ax=(Registros[10].ValorRegistro) & 0xFFFF;
    char straux[30],car[4],num1[4],num2[4];
    __int32 vector[128];

    //printf("Entrando en el sys... valor de a: %d \n Valor del DS: %d. Valor del EDX:" , *a, ds, edx);

    if (*a == 0X1){
        printf("Sys 1 \n");
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
         printf("Sys 2 \n");
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
    } else if (*a==0XF){         //sys f
        if (banderas[0]) {   //si está -b
            
            if (banderas[1])
                system("cls");     
            if (banderas[2])    //disassembler 
                MuestraCodigo();
            printf("[%d] cmd: ",Registros[5].ValorRegistro);//muestro el ip en el prompt
            fflush(stdin);    
            gets(car);

            
            
            
            if (car[0]=='p'){
                breakpoint = 1;
            } else if (car[0]>='0' && car[0]<='9'){          //si el primer caracter es un numero 
                breakpoint = 0;
                i=0;
                while (car[i]!='\0' && car[i]!=' '){    //concateno el numero en el vector num1 hasta que haya espacio o no haya nada (ya se que no sera negativo)
                    num1[i]=car[i];
                    i++;
                }
                x=atoi(num1);         //convierto num1 a integer
                if (car[i]=='\0')    //si no hay nada, entonces no hay segundo numero
                    printf("[%d]: Hexa: %X Decimal: %d \n", x, Memoria[x], Memoria[x]);
                else if (car[i]==' '){    //si hay espacio, hay segundo numero. no validé que entre otra cosa
                    j=0;
                    while(car[i]!='\0'){      //mientras no sea nulo, concateno en num2
                        num2[j]=car[i];
                        i++;
                        j++;
                    }
                    y=atoi(num2);
                    printf("%d",y);       //convierto num2 a int
                    if (y>0 && x<y){      //si y es un entero positivo printeo entre x e y.
                        for (i=x;i<=y;i++){
                            printf("[%d]: Hexa: %X  Decimal: %d\n", i, Memoria[i], Memoria[i]);
                        }
                    }
                }
            } else if (car[0]=='r'){
                breakpoint = 0;
                //printf("Continuando ejecucion...");
            }
        }
    }  else if (*a==0X3){       //string read
        cx= cx & 0xFFFF;
        ax= ax & 0xFFFF;
        printf("Sys 3 \n");
        char aux[50];
        if (!(ax & 0x800)){                            
            printf("[%d]:\t", edx+ds);
        } 
        fflush(stdin);
        scanf("%s",aux);
        i=0;
       // printf("Longitud de la palabra: %d \n", strlen(aux));
        while(i<strlen(aux) && i<cx){
        //    printf ("Cargando caracter %c \n", aux[i]);
            Memoria[edx+ds+i]=aux[i];
            i++;
        } 
        Memoria[edx+ds+i]='\0';
     } else if (*a==0x4){        //string write
         ax= ax & 0xFFFF;
         printf("Sys 4 \n");
        i=0;
        if ((!(ax & 0x100))){    //printeo con endline
            while (Memoria[edx+ds+i]!='\0'){  
                if (!(ax & 0x800)){                            
                      printf("[%d]:\t", edx+ds+i);
                }  
                printf("%c \n", Memoria[edx+ds+i]);
                i++;
            } 
        } else{   //print sin endline
            while (Memoria[edx+ds+i]!='\0'){   
                if (!(ax & 0x800)){                            
                      printf("[%d]:\t", edx+ds+i);
                } 
                printf("%c", Memoria[edx+ds+i]);
                i++;
            }
        }
    }
     else if (*a==0x7){
         system("cls"); 
     } else if (*a==0xD){
         printf("Entrando al SYS D \n");
        
         
         printf("EDX: %X \n", edx);
        __int16 DL = edx & 0xFF;  //numero de disco
        __int32 ebx = Registros[11].ValorRegistro;    //1er celda de buffer de lectura/escritura. en EH esta el sector, en EL la celda 
        __int16 EL= ebx & 0xFF;    // OFFSET del registro EH
        __int16 EH = (ebx >> 16);    //segmento donde escribir/leer (DS o ES).


        TListaDiscos aux=LD;

        while (aux!=NULL && DL!=aux->numDisco)
            aux=aux->sig;

        printf("El numero de disco es %d \n", aux->numDisco);
        if (aux==NULL){ //no existe el número disco
            printf("No existe el numero de disco \n");
            estado=0x31;
            Registros[10].ValorRegistro = (estado << 16) | (Registros[10].ValorRegistro & 0xFF);
        } else{  //encontré el número de disco
            printf("Encontré el numero de disco. \n");
            sectorDisco sec;
            FILE *Disk = fopen(aux->nombreDisco,"rb+");   //r+ permite leer y escribir. abro el disco de posicion DL
            if (Disk==NULL){       //si no existe disco con ese nombre hay que crear uno con valores default.
                Disk = fopen(aux->nombreDisco,"wb+");    //w+ crea un archivo vacio para leer y escribir
                printf("Creando archivo default!! \n");
                sectorDisco sec;
                sec.tipoArchivo = 0x56444430; // es VDD0 en hexa. comprobadísimo
                sec.numVersion=0x1;
                //strcpy(sec.GUID,"7ee914b137774e84b31387ee9bed04a2");  //randomizar despues
                strcpy(sec.GUID,"x7ee137774");
                sec.fechaCreacion=0X1348A6D;
                sec.horaCreacion=0x00A12DE1;
                sec.tipo=0x1;
                sec.cantCilindros=0x80;
                printf("La cantidad de cilindros sera de %d", sec.cantCilindros); //128 no entra en 8 bits!!!! por complemento a 2, va de -128 a 127 
                sec.cantCabezas=0x80;
                sec.cantSectores=0x80;
                sec.tamSector=0x200;
                strcpy(sec.relleno,"Relleno :)");
                fwrite(&sec,sizeof(sectorDisco),1,Disk);
                fseek(Disk,0,SEEK_SET);
            } else{
            printf("Abriendo disco %s \n", aux->nombreDisco);
            
            __int8 AH = (ax >> 8);      //numero operacion
            if (!(AH!=0x0 && AH!=0x2 && AH!=0x8 && AH!=0x3)){
                printf("Operacion valida \n");
                __int8 AL = (ax & 0xFF);     //cantidad sectores a leer/escribir
                __int8 CH = (cx >> 8);          //num cilindro
                __int8 CL = (cx & 0xFF);        //num cabeza
                __int8 DH = ((edx & 0xFFFF) >> 8);          //sector

                fread (&sec,sizeof(sectorDisco),1,Disk);
                printf("Valores disco \n Tipo archivo: %X  \n Cantidad de cilindros: %d \n Cantidad de cabezas: %d \n Cantidad de sectores: %d \n Tamanio de sector: %d \n \n", sec.tipoArchivo,sec.cantCilindros,sec.cantCabezas,sec.cantSectores,sec.tamSector);

                printf("GUID: \t");
                for (int l=0;l<16;l++){
                    printf("%X", sec.GUID[l]);
                }
                
                //fread (&sec,sizeof(sectorDisco),1,Disk);
                printf("Valores disco \n Tipo archivo: %s \n GUID: %s \n Cantidad de cilindros: %d \n Cantidad de cabezas: %d \n Cantidad de sectores: %d \n Tamanio de sector: %d \n \n", sec.tipoArchivo,sec.GUID,sec.cantCilindros,sec.cantCabezas,sec.cantSectores,sec.tamSector);


                BYTE C =sec.cantCilindros;  //cantidad cilindros    
                if (CH<=C && CH>0){
                     BYTE Ca=sec.cantCabezas;          //cant cabezas (se usa en case 8)  
                     //printf(" Ca: %d CL: %d \n", Ca, CL);   
                    if (CL<=Ca && CL>0){                //cantidad sectores
                        BYTE S=sec.cantSectores;           
                        if (DH<=S && DH>0){
                            printf("Valores validos \n");
                            __int32 TS=sec.tamSector;    //tamaño del sector en bytes  
                            int res = HD + CH*C*S*TS + CL*S*TS + DH*TS;
                            fseek(Disk,res,SEEK_SET); 
                            switch (AH)
                            {
                            case 0x0:   //consultar ultimo estado
                                switch (estado)
                                {
                                case (0x0):
                                    printf("Estado %2X: Operacion exitosa \n",estado);
                                    break;
                                case (0x1):
                                    printf("Estado %2X: Funcion invalida \n",estado);
                                    break;
                                case (0x4):
                                    printf("Estado %2X: Error de lectura \n",estado);
                                    break;
                                case (0xB):
                                    printf("Estado %2X: Numero invalido de cilindro \n",estado);
                                    break;
                                case (0xC):
                                    printf("Estado %2X: Numero invalido de cabeza \n",estado);
                                    break;
                                case (0xD):
                                    printf("Estado %2X: Numero invalido de sector \n",estado);
                                    break;
                                case (0x31):
                                    printf("Estado %2X: No existe el disco \n",estado);
                                    break;
                                case (0xCC):
                                    printf("Estado %2X: Falla de escritura \n",estado);
                                    break;
                                case (0xFF):
                                    printf("Estado %2X: Falla de operacion \n",estado);
                                    break;
                                default:
                                    printf("No se han realizado operaciones en el disco \n");
                                }
                                break;
                            case 0x2:    //leer del disco. ver errores de lectura.
                                
                                fread(&vector,sizeof(vector),1,Disk);
                                //DS empieza en Header[4]
                                if (EH == 0){      //DS
                                    __int16 DSL = (Header[4]);     //primera celda del DS. 11
                                    __int16 DSH = (Header[4]+ Header[1]); //ultima celda del DS. 1035
                                    printf("Primera celda del DS: %d. \n Ultima celda del DS: %d", DSL,DSH);
                                    for (j=0;j<AL;j++){             //itero la cantidad de sectores que me pide
                                        if (DSL+EL>=DSL && DSL+EL+128<=DSH){      //verifico que estoy en rango del DS      . ultima celda=1035
                                            for (i=0;i<128;i++){                //guardo un sector d 512 en memoria.
                                                printf("Guardando en la seccion de memoria %d \n", DSL+EL+i);
                                                Memoria[DSL+EL+i]=vector[i];
                                            }
                                        if (CH==129){       //si es el ultimo cilindro
                                                CH=1;
                                                CL++;
                                                if (CL==129){     //si es el ultimo cabezal
                                                    CL=1;
                                                    DH++;
                                                    if (DH==129){     //si es el ultimo sector
                                                        printf("Dirección inexistente!! \n");
                                                    }
                                                }
                                            } 
                                            int res = HD + CH*C*S*TS + CL*S*TS + DH*TS;
                                            fseek(Disk,res,SEEK_SET); 
                                            fread(&vector,sizeof(vector[128]),1,Disk);
                                            CH++;    
                                            DSL+=128; 
                                        }else{
                                            estado=0x04;
                                            printf("OVERFLOW!! \n");  //no deberia mostrarlo aca, sino cuando muestre el estado
                                            //exit(0);
                                        }
                                            
                                        }       
                                } 
                                //ES empieza en Header[4] + Header[1]
                                else if (EH == 2){   //ES
                                    __int16 ESL = (Header[4])+(Header[1]);  //cs + ds. esto es donde empieza el ES
                                    __int16 ESH = (Header[4])+(Header[1]) + (Header[3]); //cs + ds + es. Esto es donde termina el ES.
                                   
                                    printf("Primera celda del ES: %d. \n Ultima celda del ES: %d \n", ESL,ESH); //ESL=1035    ESH=2059
                                    for (j=0;j<AL;j++){
                                        if (ESL+EL>=ESL && ESL+EL+128<=ESH){ //1035+64+128<=2059
                                            for (i=0;i<128;i++){
                                                printf("Guardando en la seccion de memoria %d \n", ESL+EL+i);
                                                Memoria[ESL+EL+i]=vector[i];    
                                            } 
                                            if (CH==129){       //si es el ultimo cilindro
                                                CH=1;
                                                CL++;
                                                if (CL==129){     //si es el ultimo cabezal
                                                    CL=1;
                                                    DH++;
                                                    if (DH==129){     //si es el ultimo sector
                                                        printf("Dirección inexistente!! \n");
                                                    }
                                                }
                                            } 
                                            int res = HD + CH*C*S*TS + CL*S*TS + DH*TS;
                                            fseek(Disk,res,SEEK_SET); 
                                            fread(&vector,sizeof(vector[128]),1,Disk);
                                            CH++;    
                                            ESL+=128;
                                        } else{
                                            estado=0x04;
                                            printf("OVERFLOW!! \n"); //no deberia mostrarlo aca, sino cuando muestre el estado
                                            //exit(0);
                                        }
                                    }

                                } else{
                                    printf("Segmento invalido \n");
                                }
                                estado=0x0;
                                break;
                            case 0x3:    
                                if (EH == 0){      //DS
                                    __int16 DSL = (Header[4]);     //primera celda del DS. 11
                                    __int16 DSH = (Header[4]+ Header[1]); //ultima celda del DS. 1035
                                    printf("Primera celda del DS: %d. \n Ultima celda del DS \n: %d", DSL,DSH);
                                    for (j=0;j<AL;j++){             //itero la cantidad de sectores que me pide
                                        if (DSL+EL>=DSL && DSL+EL+128<=DSH){      //verifico que estoy en rango del DS      . ultima celda=1035
                                            for (i=0;i<128;i++){                //guardo un sector d 512 en memoria.
                                                printf("Leyendo de la seccion de memoria %d \n", DSL+EL+i);
                                                vector[i]=Memoria[DSL+EL+i];
                                            }
                                        if (CH==129){       //si es el ultimo cilindro
                                                CH=1;
                                                CL++;
                                                if (CL==129){     //si es el ultimo cabezal
                                                    CL=1;
                                                    DH++;
                                                    if (DH==129){     //si es el ultimo sector
                                                        printf("Dirección inexistente!! \n");
                                                    }
                                                }
                                            } 
                                            int res = HD + CH*C*S*TS + CL*S*TS + DH*TS;
                                            fseek(Disk,res,SEEK_SET); 
                                            fwrite(&vector,sizeof(vector[128]),1,Disk);
                                            CH++;    
                                            DSL+=128; 
                                        }else{
                                            estado=0xCC;
                                            printf("OVERFLOW!! \n");  //no deberia mostrarlo aca, sino cuando muestre el estado
                                          //  exit(0);
                                        }
                                            
                                        }       
                                } 
                                //ES empieza en Header[4] + Header[1]
                                else if (EH == 2){   //ES
                                    __int16 ESL = (Header[4])+(Header[1]);  //cs + ds. esto es donde empieza el ES
                                    __int16 ESH = (Header[4])+(Header[1]) + (Header[3]); //cs + ds + es. Esto es donde termina el ES.
                                    printf("Primera celda del ES: %d. \n Ultima celda del ES: %d \n", ESL,ESH); //ESL=1035    ESH=2059
                                    for (j=0;j<AL;j++){
                                        if (ESL+EL>=ESL && ESL+EL+128<=ESH){ //1035+64+128<=2059
                                            for (i=0;i<128;i++){
                                                printf("Leyendo de la seccion de memoria %d \n", ESL+EL+i);
                                                vector[i]=Memoria[ESL+EL+i];   
                                            } 
                                            if (CH==129){       //si es el ultimo cilindro
                                                CH=1;
                                                CL++;
                                                if (CL==129){     //si es el ultimo cabezal
                                                    CL=1;
                                                    DH++;
                                                    if (DH==129){     //si es el ultimo sector
                                                        printf("Dirección inexistente!! \n");
                                                    }
                                                }
                                            } 
                                            int res = HD + CH*C*S*TS + CL*S*TS + DH*TS;
                                            fseek(Disk,res,SEEK_SET); 
                                            fwrite(&vector,sizeof(vector[128]),1,Disk);
                                            CH++;    
                                            ESL+=128;
                                        } else{
                                            estado=0xCC;
                                            printf("OVERFLOW!! \n");  //no deberia mostrarlo aca, sino cuando muestre el estado
                                          //  exit(0);
                                        }
                                    }

                                } 
                                break;

                            case 0x8:    //obtener parametros
                                Registros[12].ValorRegistro = C;
                                Registros[12].ValorRegistro = (Registros[12].ValorRegistro << 8) | Ca;   //devuelvo cant cilindros en CH y cant cabezas en CL
                                Registros[13].ValorRegistro = S;
                                Registros[13].ValorRegistro = Registros[13].ValorRegistro << 8;  //devuelvo cant sectores en DH
                                estado=0x0;
                                break;
                            
                            default:
                                break;
                            }
                        } else {
                            printf("Sectores invalidos!! \n");
                            estado=0x0D; //sector invalido
                            Registros[10].ValorRegistro = (estado << 16) | (Registros[10].ValorRegistro & 0xFF);  //devuelvo el error en AH
                        }
                    } else {
                        printf("Cabeza invalida! \n");
                        estado=0x0C;  //cabeza invalida
                        Registros[10].ValorRegistro = (estado << 16) | (Registros[10].ValorRegistro & 0xFF);
                    }
                } else{
                    printf("Cilindro invalido!!");
                    estado=0x0B;  //cilindro invalido
                    Registros[10].ValorRegistro = (estado << 16) | (Registros[10].ValorRegistro & 0xFF);
                }
            } else
                estado=0x01;  //funcion invalida
            }
            }
        } 
     }


void stop(){
    Registros[5].ValorRegistro = Registros[0].ValorRegistro;
}

/*

void SetParteAlta(int i, int32_t valor){
    valor = (valor << 16);
    Registros[i].ValorRegistro = valor + GetParteBaja(i);
}

void SetParteBaja(int i, int32_t valor){
    Registros[i].ValorRegistro = (GetParteAlta(i) << 16) + (valor & 0x0000FFFF);
}

int32_t GetParteAlta(int i){
    int32_t alta = Registros[i].ValorRegistro;
    alta = (alta >> 16);
    return alta;
}

int32_t GetParteBaja(int i){
    int32_t baja = Registros[i].ValorRegistro;
    baja = (baja << 16);
    baja = (baja >> 16);
    return baja;
}
*/

//-------------------------PILA-------------------------

void push(__int32 *a){
    int SPL = Registros[6].ValorRegistro & 0xFFFF; //Tope de la pila
    if (SPL == 0) // Stack Overflow.
       Errores[3] = 1; 
    else{
        Registros[6].ValorRegistro-=1;
        //SetParteBaja(6, SPL - 1); Siempre guarda en la parte de arriba de la pila y decrementa el SPL.
        SPL--;
        Memoria[Registros[1].ValorRegistro & 0XFFFF + SPL] = (*a); //GetParteBaja(1) = direccion del SS.
    }
}

void pop(__int32 *a){
    int SPL = Registros[6].ValorRegistro & 0XFFFF; //GetParteBaja(6);
    if(SPL > Registros[1].ValorRegistro >> 16)// Direccion SS + SPL
       Errores[4] = 1;// detiene_ejecucion=3; //Stack Underflow Este es otro error 
    else{
        (*a) = Memoria[Registros[1].ValorRegistro & 0XFFFF + SPL];
        Registros[6].ValorRegistro+=1; //SetParteBaja(6, SPL + 1);
    }
}

void call(__int32 *a){
    __int32 IP = Registros[5].ValorRegistro; //El IP ya fue incrementado antes en ejecución.
    push(&IP);
    if(!Errores[3]) //El IP se pusheó bien
        Registros[5].ValorRegistro = *a; //JMP operando
}

void ret(){
    __int32 IP;
    pop(&IP); //Toma la dirección de la pila.
    if(!Errores[4]) //Si no hubo Underflow luego del pop.
        Registros[5].ValorRegistro=IP; //Le asigna la dirección al registro IP.
}





//-----------------------------------------MV----------------------------------------------------------

int main(int arg,char *args[]){
    
    
    FILE *archI;
    archI = fopen(args[1],"rb");
    fread(Header,sizeof(__int32),6,archI);
    fread(Memoria,sizeof(__int32),Header[4],archI);

    //Verifica Si la primer celda tiene MV-2 y la ultima V.22 --Error1 a detectar en ejecucion 
    if(!(Header[0] == 0x4D562D31 && Header[5] == 0x562E3232)){
         Errores[0] = 1;       
    }
    else if(!((Header[1]+Header[2]+Header[3]) <= (cantidadMR - Header[4]))){
        Errores[1] = 1;    
    }
    
    if(!(Errores[0] || Errores[1])){
        cargaMnemonicos();
        InicializaRegistros();
        
        FILE *archDiscos;
        TListaDiscos discos=NULL;
        archDiscos = fopen(args[2],"rwb");
        
        TListaDiscos aux;
        char disco[30];
        int p=0;
        while (fscanf(archDiscos,"%s",disco)==1){
            aux= (TListaDiscos) malloc (sizeof(nodo));
            strcpy(aux->nombreDisco,disco);
            aux->sig=LD;
            aux->numDisco=p;
            p++;
            LD=aux;
        }

        aux=LD;

        while (aux!=NULL)
        {
            printf("Disco %s \n", aux->nombreDisco);
            aux=aux->sig;
        }

        if(archI!=NULL){    
            if (arg>2){ //Si hay banderas, se fija cuáles están.
                for (int i=3; i < arg; i++){
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
    else{
        if(Errores[0]) 
            printf("El formato del archivo %s no es correcto", args[1]);
        else if(Errores[1])   
            printf("Memoria insuficiente");
    }    

   
    return 0;
}

void InicializaRegistros(){
    strcpy(Registros[0].nombre,"DS "); Registros[0].ValorRegistro =  Header[4] | (Header[1]<<16); 
    strcpy(Registros[1].nombre,"SS "); Registros[1].ValorRegistro = (Header[1] + Header[4] + Header[3]) | (Header[2]<<16);
    strcpy(Registros[2].nombre,"ES "); Registros[2].ValorRegistro = (Header[1] + Header[4]) | (Header[3]<<16);
    strcpy(Registros[3].nombre,"CS "); Registros[3].ValorRegistro = Header[4]<<16;
    strcpy(Registros[4].nombre,"HP "); Registros[4].ValorRegistro= 0x00020000;
    strcpy(Registros[5].nombre,"IP "); Registros[5].ValorRegistro= 0X00000000;
    strcpy(Registros[6].nombre,"SP "); Registros[6].ValorRegistro= 0x00010000 | (Registros[1].ValorRegistro>>16 && 0xFFFF); 
    strcpy(Registros[7].nombre,"BP "); Registros[7].ValorRegistro= 0x00010000;
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

    mnemonicos[12].cod=0xC;
    strcpy(mnemonicos[12].mnemonico,"SLEN");
    mnemonicos[13].cod=0xD;
    strcpy(mnemonicos[13].mnemonico,"SMOV");
    mnemonicos[14].cod=0xE;
    strcpy(mnemonicos[14].mnemonico,"SCMP");
    mnemonicos[15].cod=0XF0;
    strcpy(mnemonicos[15].mnemonico,"SYS");
    mnemonicos[16].cod=0XF1;
    strcpy(mnemonicos[16].mnemonico,"JMP");
    mnemonicos[17].cod=0XF2;
    strcpy(mnemonicos[17].mnemonico,"JZ");
    mnemonicos[18].cod=0XF3;
    strcpy(mnemonicos[18].mnemonico,"JP");
    mnemonicos[19].cod=0XF4;
    strcpy(mnemonicos[19].mnemonico,"JN");
    mnemonicos[20].cod=0XF5;
    strcpy(mnemonicos[20].mnemonico,"JNZ");
    mnemonicos[21].cod=0XF6;
    strcpy(mnemonicos[21].mnemonico,"JNP");
    mnemonicos[22].cod=0XF7;
    strcpy(mnemonicos[22].mnemonico,"JNN");
    mnemonicos[23].cod=0XF8;
    strcpy(mnemonicos[23].mnemonico,"LDL");
    mnemonicos[24].cod=0XF9;
    strcpy(mnemonicos[24].mnemonico,"LDH");
    mnemonicos[25].cod=0XFA;
    strcpy(mnemonicos[25].mnemonico,"RND");
    mnemonicos[26].cod=0XFB;
    strcpy(mnemonicos[26].mnemonico,"NOT");
    mnemonicos[27].cod = 0xFC;
    strcpy(mnemonicos[27].mnemonico, "PUSH");
    mnemonicos[28].cod = 0xFD;
    strcpy(mnemonicos[28].mnemonico, "POP");
    mnemonicos[29].cod = 0xFE;
    strcpy(mnemonicos[29].mnemonico, "CALL");
    mnemonicos[30].cod = 0xFF0;
    strcpy(mnemonicos[30].mnemonico, "RET");
    mnemonicos[31].cod = 0xFF1;
    strcpy(mnemonicos[31].mnemonico, "STOP");
    
}

 long decodificaString(__int32 op, __int32 tipoOp){
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
        valorOp = op+(Registros[0].ValorRegistro & 0xFFFF);
    } else if (tipoOp == 3){   //indirecto.
        __int8 offset = valorOp >> 4;                                   //offset
        __int8 codReg = valorOp & 0xF;                                 //numero de registro que viene de la traduccion
        __int32 codSeg = Registros[codReg].ValorRegistro >> 16;        // codigo del segmento al que referenciaré
        __int32 seg = Registros[codSeg].ValorRegistro & 0xFFFF;        //donde comienza el segmento en memoria
        __int32 tamSeg = Registros[codSeg].ValorRegistro >> 16;        //tamaño del segmento referenciado
        __int32 valorRegistro = Registros[codReg].ValorRegistro & 0xFFFF;   //valor registro

       /* printf("Codigo registro: %X \n", codReg);
        printf("Codigo de segmento: %d Tamano del segmento: %d \n", codSeg, tamSeg);
        printf("El segmento comienza en la celda %d \n", seg);
        printf("El valor de mi registro es de %d \n", valorRegistro); */

        if (seg+valorRegistro+offset<=tamSeg+seg){
            //printf("La posicion de memoria es: %d y el valor es: %d",seg+valorRegistro+offset, seg+valorRegistro+offset);
            valorOp= seg+valorRegistro+offset;
        } else{
            Errores[2]=1;
        }
    }
    return valorOp; // si es inmediato lo devuelve igual
} 

void leeInstruccion(){
    __int32 sysB=0XF;
    int cantidadOperandos=0;
    __int32 instruccion,mnemonico,tipoOp1,tipoOp2,op1,op2,valorOp1,valorOp2,sectorOp2;
    void (*fun[])(__int32 *, __int32 *) = {mov, add, sub,swap,mul,DIV,cmp,shl,shr,and,or,xor,slen,smov,scmp};
    void (*fun2[])(__int32 *) = {sys,jmp, jz,jp,JN,jnz,jnp,jnn,ldl,ldh,rnd,not,push,pop,call};

    while (Registros[5].ValorRegistro >=0 && Registros[5].ValorRegistro<(Registros[0].ValorRegistro & 0xFFFF) && !(Errores[2] || Errores[3] || Errores[4])){

        instruccion = Memoria[Registros[5].ValorRegistro];
        printf("%08X",instruccion);
        Registros[5].ValorRegistro++;
        
        mnemonico = leeMnemonico(instruccion,&cantidadOperandos);
       // printf("Cod Mnemonico: %d\n", mnemonico);
        if(cantidadOperandos == 2){
            tipoOp1 = (instruccion >> 26) & 0X3;
            tipoOp2 = (instruccion >> 24) & 0X3;
            op1 = (instruccion >> 12) & 0XFFF;
            op2 = instruccion & 0XFFF;      
            if (mnemonico!=0xC && mnemonico!=0xD && mnemonico!=0xE){
                valorOp1 = decodificaOperando(op1,tipoOp1);
                valorOp2 = decodificaOperando(op2,tipoOp2);
            } else{
                valorOp1 = decodificaString(op1,tipoOp1);
                valorOp2 = decodificaString(op2,tipoOp2);

            } 
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
            if(mnemonico!=0X6 && mnemonico != 0xE && mnemonico != 0xD){ // alamcena los valores calculados anteriormente en los registros o memoria correspondiente menos en el cmp 
                alamacenaRM(valorOp1,tipoOp1,op1);
                if(op1 != op2 && mnemonico != 0xC)
                    alamacenaRM(valorOp2,tipoOp2,op2);
            }
            if(mnemonico != 0X0 && mnemonico != 0X3 && mnemonico !=0X6 && mnemonico != 0XE && mnemonico != 0XD && mnemonico!= 0XC){ // cambia el valor de CC seguun el resultado que se calcule
                cambiaCC(valorOp1);
            }
            

        } else if(cantidadOperandos == 1){
            tipoOp1 = (instruccion>>22) & 0X3;
            op1 = instruccion & 0XFFFF; 
            valorOp1 = decodificaOperando(op1,tipoOp1);
            (*fun2[instruccion>>24 & 0XF])(&valorOp1);
            if (mnemonico==0XFA || mnemonico==0XFB){   // RND, NOT
                alamacenaRM(valorOp1,tipoOp1,op1);
            }
            if (mnemonico==0XFB){
                cambiaCC(valorOp1);
            }
        } else {
            if(mnemonico == mnemonicos[31].cod)
                stop();
            else if (mnemonico == mnemonicos[30].cod)
                ret();
                  
        }
        if(breakpoint == 1 && mnemonico != mnemonicos[15].cod){
            
            sys(&sysB);
        }
    }
    if(Errores[0]) 
        printf("Segmentation Fault");
    else if(Errores[1])   
        printf("Stack Overflow");
    else if(Errores[4])
        printf("Stack Underflow");

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
        //cambiar considerando los distintos segmentos
        __int32 direccion = (Registros[0].ValorRegistro & 0xFFFF) + op;
        if((direccion >= Registros[0].ValorRegistro & 0xFFFF) && direccion <= )
            valorOp = Memoria[op+Registros[0].ValorRegistro & 0xFFFF];
    } else if (tipoOp == 3){   //indirecto.
        __int8 offset = valorOp >> 4;                                   //offset
        __int8 codReg = valorOp & 0xF;                                 //numero de registro que viene de la traduccion
        __int32 codSeg = Registros[codReg].ValorRegistro >> 16;        // codigo del segmento al que referenciaré
        __int32 seg = Registros[codSeg].ValorRegistro & 0xFFFF;        //donde comienza el segmento en memoria
        __int32 tamSeg = Registros[codSeg].ValorRegistro >> 16;        //tamaño del segmento referenciado
        __int32 valorRegistro = Registros[codReg].ValorRegistro & 0xFFFF;   //valor registro

     /*   printf("Codigo registro: %X \n", codReg);
        printf("Codigo de segmento: %d Tamano del segmento: %d \n", codSeg, tamSeg);
        printf("El segmento comienza en la celda %d \n", seg);
        printf("El valor de mi registro es de %d \n", valorRegistro); */
        if(seg >= 0 && seg<=3){
            if (seg+valorRegistro+offset<=tamSeg+seg){
                //printf("La posicion de memoria es: %d y el valor es: %d",seg+valorRegistro+offset, Memoria[seg+valorRegistro+offset]);
                valorOp= Memoria[seg+valorRegistro+offset];
            } else{
                Errores[2];
            }
        }
        else{    
            Errores[2];
        }
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
        Memoria[op+(Registros[0].ValorRegistro&0xFFFF)] = valorOp;
    } else if(tipoOp==3){ //indirecto
        __int8 offset = op >> 4;                                   //offset
        __int8 codReg = op & 0xF;                                 //numero de registro que viene de la traduccion
        __int32 codSeg = Registros[codReg].ValorRegistro >> 16;        // codigo del segmento al que referenciaré
        __int32 seg = Registros[codSeg].ValorRegistro & 0xFFFF;        //donde comienza el segmento en memoria
        Memoria[(Registros[codReg].ValorRegistro & 0XFFFF) + seg + offset] =  valorOp;

    }
}

void MuestraCodigo(){
    printf("Codigo:");
    for (int i = 0; i < (Registros[0].ValorRegistro & 0xFFFF); i++)
        Dissasembler(i);
    //MUESTRA DE REGISTROS
    printf("\nRegistros:\n");
    for(int j = 0; j < 16; j++){
        printf("%3s = %08X |", Registros[j].nombre, Registros[j].ValorRegistro);
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
        printf("%s\t\t",mnemonicos[nroMnemonico + 30].mnemonico); //Mnemónico
    }
    else if((inst & 0xF0000000) == 0xF0000000){ //Si es de un operando
        nroMnemonico= ((inst&0x0F000000)>>24);
        printf("%s\t\t",mnemonicos[nroMnemonico + 15].mnemonico); //Mnemónico
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
        else { //Supongo inmediato  //else PONER INDIRECTOS
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
        else { // Si operando 1 es inmediato  //else PONER INDIRECTOS
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
        } //else PONER INDIRECTOS
    }
}