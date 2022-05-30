#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <ctype.h>
#include "parser.h"

typedef struct TRegistros{
    __int32 ValorRegistro;
    char nombre[3];
} TRegistros;


typedef struct nodoS{
    char nombre[20],valor[20];
    struct nodoS *sig;
} nodoS;

typedef struct nodoS *ListaString;

typedef struct {
    char mnemonico[5];
    __int32 cod;
} elementosMnemonicos;

typedef struct nodo {
    char rotulo[10];
    __int32 linea;
    struct nodo *sig;
}   nodo;

typedef struct nodo* ListaRotulos;
int hexa_u_octal;
 __int32 Memoria[8192]={0};
int error = 0; //cuando hay un error se pone en 1

//------------------------------------------TRADUCTOR MV----------------------------------------------------------
int rotuloInmediato(ListaRotulos LR, char operando[]);
__int32 tipoOperando(char op[],ListaRotulos LR);
__int32 transformaOperando(char operando[],int tipoOperando,TRegistros Registros[],ListaRotulos LR);
__int32 DevuelveInmediato(char operando[],ListaRotulos LR);
__int32 DevuelveDirecto(char operando[]);
__int32 DevuelveRegistro(char operando[],TRegistros Registros[]);
__int32 esRotulo(char ope[],ListaRotulos LR);
__int32 buscaRegistro(char operando[],TRegistros Registros[]);
void InicializaRegistros(TRegistros Registros[]);
void buscaRotulo(ListaRotulos *LR, FILE *archA, int *tamanoCS, int *tamanoDS, int *tamanoES, int *tamanoSS, ListaString *LS);
void parseo(FILE *archA,char **parsed);
void Traduccion(char **parsed,__int32 *instruccionBin,elementosMnemonicos mnemonicos[],ListaRotulos LR,TRegistros Registros[],int *error,int i,char *imprimir);
void InicializaHeader(__int32 Header[],int tamanoCS ,int tamanoDS, int tamanoES, int tamanoSS);
void cargaMnemonicos(elementosMnemonicos mnemonicos[]);
void ingresarRotulo(ListaRotulos *LR, char* rotulo, int lineaRotulo);
__int32 recorreMnemonicos(elementosMnemonicos mnemonicos[],char* mnemonico);
void strToUpper(char palabra[]);
__int32 DevuelveConstantValue(char operando[]);
__int32 DevuelveIndirecto(char operando[],TRegistros Registros[], ListaRotulos LR);
int duplicado(ListaRotulos LR, char nombreSimbolo[20]);
int duplicadoStr(ListaString LS, char nombreSimbolo[20]);

int main(int arg, char *args[])
{   
   

    int i=0,j=6,tamanoCS=0 ,tamanoDS=1024, tamanoES=1024, tamanoSS=1024;    //checkear cuando hay que sumar el numero de linea y cuando no
    __int32 header[6];
    elementosMnemonicos mnemonicos[32];
    ListaRotulos LR = NULL;
    ListaString LS=NULL;
    __int32 instruccionBin=0X0;
    char imprimir[3];
    FILE *archI;
    FILE *archO;
    archI=fopen(args[1],"rt");  //se abre el archivo de entrada para leer el programa assembler
    archO = fopen(args[2],"wb"); //se abre el archivo de salida de la traduccion para escritura en binario

    char **parsed;
    TRegistros Registros[16];

    //Inicializamos las estructuras 
    
    cargaMnemonicos(mnemonicos);
    InicializaRegistros(Registros);

    //-------------------------------------------------------
    if(arg==4){
        strcpy(imprimir,args[3]);
    } else {
        strcpy(imprimir,"-null");
    }

    if(archI!=NULL)     //si el archivo de entrada no existe o se genera algun error no hace nada
    {
        buscaRotulo(&LR,archI,&tamanoCS,&tamanoDS,&tamanoES,&tamanoSS,&LS);

        while(LS!=NULL){      //inserto constantes string despues del CS
            int i=0;
            while (i<=strlen(LS->valor)){
                if (i==0){
                int direccion=tamanoCS+1;
                ListaRotulos aux;
                aux= (ListaRotulos) malloc (sizeof(nodo));
                aux->linea=direccion;
                strcpy(aux->rotulo,LS->nombre);
                aux->sig=LR;
                LR=aux;
            }
                Memoria[++tamanoCS]=LS->valor[i];
                i++;
            }
            LS=LS->sig;
        }
       // Registros[0].ValorRegistro=contadorDS;
        
        printf("El tamanio del CS es de: %d \n", tamanoCS);
        InicializaHeader(header,tamanoCS,tamanoDS,tamanoES,tamanoSS);
        fseek(archI,0,SEEK_SET);

            for (int i=0; i < 6; i++){
                Memoria[i] = header[i];
            }
      
      
        


        while (!feof(archI)){   
            char instruccionAss[256];
            fgets(instruccionAss,256,archI);
              //lee la linea correspondiente del archivo asm
            parsed = parseline(instruccionAss);
            int lineaVacia = parsed[0]==NULL && parsed[1]==NULL && parsed[2]==NULL && parsed[3]==NULL;
            if((!lineaVacia)){
                Traduccion(parsed,&instruccionBin,mnemonicos,LR,Registros,&error,i,imprimir);
                Memoria[j++]=instruccionBin;
                i++;
            } else {
                if(parsed[4]!=NULL && strcmp(imprimir,"-o")==0)
                    printf("%s\n",parsed[4]);
                }      
        }
        freeline(parsed);
      
        if(error==0){
            fwrite(Memoria,sizeof(__int32),j,archO);   
        }       
    }
    fclose(archO);
    fclose(archI);

    return 0;

}

void InicializaHeader(__int32 Header[],int tamanoCS ,int tamanoDS, int tamanoES, int tamanoSS){
    Header[0] = 0x4D562D31;
    Header[1] = tamanoDS; //Corresponde al DS
    Header[2] = tamanoSS; 
    Header[3] = tamanoES; 
    Header[4] = tamanoCS;
    Header[5] = 0x562E3232;
}

void InicializaRegistros(TRegistros Registros[]){
    strcpy(Registros[0].nombre,"DS "); Registros[0].ValorRegistro=0;
    strcpy(Registros[1].nombre,"SS "); Registros[1].ValorRegistro=0;
    strcpy(Registros[2].nombre,"ES "); Registros[2].ValorRegistro=0;
    strcpy(Registros[3].nombre,"CS "); Registros[3].ValorRegistro=0;
    strcpy(Registros[4].nombre,"HP "); Registros[4].ValorRegistro=0;
    strcpy(Registros[5].nombre,"IP "); Registros[5].ValorRegistro=0;
    strcpy(Registros[6].nombre,"SP "); Registros[6].ValorRegistro=0;
    strcpy(Registros[7].nombre,"BP "); Registros[7].ValorRegistro=0;
    strcpy(Registros[8].nombre,"CC "); Registros[8].ValorRegistro=0;
    strcpy(Registros[9].nombre,"AC "); Registros[9].ValorRegistro=0;
    strcpy(Registros[10].nombre,"EAX"); Registros[10].ValorRegistro=0;
    strcpy(Registros[11].nombre,"EBX"); Registros[11].ValorRegistro=0;
    strcpy(Registros[12].nombre,"ECX"); Registros[12].ValorRegistro=0;
    strcpy(Registros[13].nombre,"EDX"); Registros[13].ValorRegistro=0;
    strcpy(Registros[14].nombre,"EEX"); Registros[14].ValorRegistro=0;
    strcpy(Registros[15].nombre,"EFX"); Registros[15].ValorRegistro=0;
}

void cargaMnemonicos(elementosMnemonicos mnemonicos[])  //funcion que carga los mnemonicos con sus respectivos codigos en un arreglo
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

void ingresarRotulo(ListaRotulos *LR, char rotulo[], int lineaRotulo)
{
    ListaRotulos aux;
    aux = (ListaRotulos) malloc(sizeof(nodo));
    aux->linea=lineaRotulo;
    aux->sig = NULL;
    strcpy(aux->rotulo,rotulo);

    //printf("Ingresando rotulo %s con valor %d \n", rotulo,lineaRotulo);

    if (*LR!=NULL)          //Ingresamos el rotulo siempre por cabecera
        aux->sig = *LR;
    *LR = aux;
}

__int32 recorreMnemonicos(elementosMnemonicos mnemonicos[],char* mnemonico)
{ 
    int i=0;
    strToUpper(mnemonico);

    while (strcmp(mnemonicos[i].mnemonico,mnemonico)!=0 && i<32){    //Busca el mnemonico en la lista de mnemonicos
        i++;
    }
    if(strcmp(mnemonicos[i].mnemonico,mnemonico)==0) {   //Si encuentra el mnemonico devuelve el codigo para la instruccion
        return mnemonicos[i].cod;
    }
    else {
        return 0xFFFFFFFF;      //Si no lo encuentra es que no existe 
    } 
    
}

void strToUpper(char palabra[]){      //Pasa a mayusculas el string que le mandas por parametro

    for(int i=0;i<strlen(palabra);i++)
    {
         palabra[i]=toupper(palabra[i]);
    }

}

void buscaRotulo(ListaRotulos *LR, FILE *archA, int *tamanoCS, int *tamanoDS, int *tamanoES, int *tamanoSS, ListaString *LS){
    char instruccionAss[256];
    char **parsed;
    int valueConst;
    int i=0;

    while(!feof(archA)){
        fgets(instruccionAss,256,archA);  //lee la linea correspondiente del archivo asm
        parsed = parseline(instruccionAss);
        if(parsed[0]!=NULL){
            char auxrotulo[20];
            strcpy(auxrotulo,parsed[0]);
            if (!duplicado(*LR,auxrotulo) && !duplicadoStr(*LS,auxrotulo))
                ingresarRotulo(LR,parsed[0],i);
            else{
                printf("ERROR: Rotulo %s duplicado. Traduccion detenida\n");
                exit(0);
            }
        }
        if(!(parsed[0]==NULL && parsed[1]==NULL && parsed[2]==NULL && parsed[3]==NULL))
            i++;
        
        if (parsed[5]!=NULL){
        strToUpper(parsed[5]);
        if(strcmp(parsed[5],"DATA")==0)
            *tamanoDS = atoi( parsed[6]);
        if(strcmp(parsed[5],"EXTRA")==0)
            *tamanoES = atoi( parsed[6]);
        if(strcmp(parsed[5],"STACK")==0)
            *tamanoSS = atoi( parsed[6]);

        printf("\n Tamaños asignados");

        }

        if(parsed[7]!=NULL && parsed[8]!=NULL){
            char aux[20];
            char auxnombre[20];
            strToUpper(parsed[7]);
            strcpy (aux,parsed[8]);
            strcpy (auxnombre,parsed[7]);
            if ((auxnombre[0]<'0' || auxnombre[0]>'9') && strlen(auxnombre)>=3 && strlen(auxnombre)<=10){    //verifico que el nombre del simbolo tenga mas de 3 y menos de 10 caracteres. y que el primer caracter no sea un digito
                if (aux[1]!='\0' && (aux[0]<'0' || aux[0]>'9' || aux[0]!='@' || aux[0]!='%' || aux[0]!='#')){    //si el valor del simbolo es un string
                    if (!duplicado(*LR,auxnombre) && !duplicadoStr(*LS,auxnombre)){
                        strToUpper(parsed[8]);
                        ListaString aux;
                        aux= (ListaString) malloc (sizeof(nodoS));
                        strcpy(aux->nombre,parsed[7]);
                        strcpy(aux->valor,parsed[8]);
                        aux->sig=*LS;      //crear lista string
                        *LS=aux;
                    } else{
                        printf("ERROR: Simbolo string %s duplicado. Traduccion detenida \n", auxnombre);
                        error=1;
                    }
                } else{      //no es string
                    if (!duplicado(*LR,auxnombre) && !duplicadoStr(*LS,auxnombre)){
                     // printf("Cargando simbolo no string %s \n", auxnombre);
                      ingresarRotulo(LR,parsed[7],DevuelveConstantValue(parsed[8]));   
                    }
                    else{
                        printf("ERROR: Simbolo %s duplicado. Deteniendo traduccion \n", auxnombre);
                        error=1;
                    }
                }
            }else {
                printf("ERROR: Simbolo %s invalido. Traduccion detenida \n", auxnombre);    //aca no se si ponerle un warning y que siga traduciendo o que cancele de una xd
                error=1;
            }
        }
        freeline(parsed);     
    }
    *tamanoCS = i;
}

int duplicadoStr(ListaString LS, char nombreSimbolo[20]){
    ListaString aux = LS;
    while (aux!=NULL && strcmp(nombreSimbolo,aux->nombre)!=0)
        aux=aux->sig;
    
    return aux!=NULL;     //si no se cayó es xq hay duplicado
}

int duplicado(ListaRotulos LR, char nombreSimbolo[20]){
    ListaRotulos aux = LR;
    while (aux!=NULL && strcmp(nombreSimbolo,aux->rotulo)!=0)
        aux=aux->sig;
    
    return aux!=NULL;     //si no se cayó es xq hay duplicado
}

__int32 esRotulo(char ope[],ListaRotulos LR){
        
    while (LR!=NULL && strcmp(ope,LR->rotulo)!=0)
        LR = LR->sig;
    if (LR!=NULL && strcmp(ope,LR->rotulo)==0)  //si lo encontré
        return LR->linea;
    else
        return 0XFFF; // ERROR: no se encuentra el rotulo
}

void Traduccion(char **parsed,__int32 *instruccionBin,elementosMnemonicos mnemonicos[],ListaRotulos LR,TRegistros Registros[],int *error,int i,char *imprimir){
    
    //VERIFICAR EL ERROR DE SINTAXIS CUANDO NO ENCUENTRA UN MNEMONICO
    __int32 mnemonico,op1,op2;
    int tipoOpe1,tipoOpe2;
    //error=0;
    char comentario[]=";";
        if(parsed[4]!=NULL){
            strcat(comentario,parsed[4]); 
        } else {
            comentario[0]=' ';
        }
  
    mnemonico=recorreMnemonicos(mnemonicos,parsed[1]);
    if(mnemonico!=0XFFFFFFFF && mnemonico!=0xFF1){ //si no hay error sigue con la ejecucion de la traduccion normal
        if(mnemonico<=0XE){
            char op1String[15], op2String[15];
            strcpy(op1String,parsed[2]);
            strcpy(op2String,parsed[3]);

            tipoOpe1 = tipoOperando(op1String,LR);
            tipoOpe2 = tipoOperando(op2String,LR);

            op1 =transformaOperando(op1String,tipoOpe1,Registros,LR);
            if (tipoOpe1==0){
                if(op1>0XFFF){
                    op1 = op1 & 0XFFF;
                    printf("WARNING: operando truncado \n");
                }
            }  
            op2 =transformaOperando(op2String,tipoOpe2,Registros,LR);

            if (tipoOpe2==0){
                if(op2>4095 && hexa_u_octal){
                    op2 = op2 & 0XFFF;
                    printf("WARNING: operando truncado \n");
                }
                if((op2>2047 || op2<-2048) && !hexa_u_octal){
                    op2 = op2 & 0XFFF;
                    printf("WARNING: operando truncado \n");
                } else if(op2 < 0 && op2>-2048){
                    op2 = op2 & 0XFFF;
                }
            } 
            *instruccionBin = (mnemonico<<28) | ((tipoOpe1<<26) & 0x0C000000) | ((tipoOpe2<<24) & 0x03000000) | ((op1<<12)) | (op2);

            char comentario[]=";";
            if(parsed[4]!=NULL){
                strcat(comentario,parsed[4]); 
            } else {
                comentario[0]=' ';
            }
            if(strcmp(imprimir,"-o")==0)
                if (parsed[0]!=NULL){
                    printf("[%04d]:\t%02X %02X %02X %02X\t\t%s: %s\t\t%s, %s\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF ,parsed[0],parsed[1],parsed[2],parsed[3],comentario);
                }else
                    printf("[%04d]:\t%02X %02X %02X %02X\t\t%d: %s\t\t%s, %s\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF ,i+1,parsed[1],parsed[2],parsed[3],comentario);
    

            //Bloque de dos operandos
        }else if (mnemonico<=0XFB){
            char op1String[8];
            strcpy(op1String,parsed[2]);
            tipoOpe1 = tipoOperando(op1String,LR);
            op1 =transformaOperando(op1String,tipoOpe1,Registros,LR);

            if(op1==0XFFF)
                *error=1;
            
            if (tipoOpe1==0){
                if(op1>0XFFFF){
                    op1 = op1 & 0XFFFF;
                    printf("WARNING: operando truncado \n");
                }
            }
            *instruccionBin = ((mnemonico << 24) & 0XFF000000) | ((tipoOpe1 << 22) & 0x00C00000) | (op1);

            char comentario[]=";";
            if(parsed[4]!=NULL){
                strcat(comentario,parsed[4]); 
            } else {
                comentario[0]=' ';
            }
            if(strcmp(imprimir,"-o")==0)
                if (parsed[0]!=NULL){
                    printf("[%04d]:\t%02X %02X %02X %02X\t\t%s: %s\t\t%s\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF ,parsed[0],parsed[1],parsed[2],comentario);
                }else
                    printf("[%04d]:\t%02X %02X %02X %02X\t\t%d: %s\t\t%s\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF ,i+1,parsed[1],parsed[2],comentario);
            //Bloque de 1 operando
        }

        }else {
            if(mnemonico==0XFFFFFFFF){
                *error=1;
                *instruccionBin = mnemonico;
                printf("La siguiente instruccion tiene un error de sintaxis: \n");
            }
            else
                *instruccionBin = (mnemonico<<20) & 0XFFF00000;
        
        if(strcmp(imprimir,"-o")==0)
            if (parsed[0]!=NULL)
                printf("[%04d]:\t%02X %02X %02X %02X\t\t%s: %s\t\t\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF, parsed[0],parsed[1],comentario);
            else
                printf("[%04d]:\t%02X %02X %02X %02X\t\t%d: %s\t\t\t\t%s\n",i,(*instruccionBin>>24) & 0XFF,(*instruccionBin>>16)&0XFF,(*instruccionBin>>8)&0XFF,*instruccionBin & 0XFF, i+1,parsed[1],comentario);
    }
     
    

}
__int32 DevuelveInmediato(char operando[], ListaRotulos LR){
        char *ope;
        hexa_u_octal=0;
        if(operando[0]== '-' || (operando[0] >= '0' && operando[0] <= '9'))
            return atoi(operando);
        else{
            if(operando[0]>=65 && operando[0]<=90 || (operando[0]>=97 && operando[0]<=122)){
                return esRotulo(operando,LR);
            } else {
                switch(operando[0]){ //Con esto vamos a devolver el valor en decimal.
                    case '#':
                        ope = &operando[1];

                        return atoi(ope);
                        break;
                    case 39: //Si es una letra.
                        
                        return (int) operando[1];
                        break;
                    case '@':
                    hexa_u_octal=1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 8);
                        break;
                    case '%':
                        hexa_u_octal=1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 16);
                        break;
                    default:
                        return -1;  // Utilizamos -1 para decir que es un error de sintaxis 
                }
            }
        }
    }

__int32 buscaRegistro(char operando[],TRegistros Registros[]){
    int i = 0;

    while (i <= 15 && strcmp(Registros[i].nombre,operando)!=0){
        i++;
    }
        

    if (strcmp(Registros[i].nombre,operando)==0)
        return i;
    else
        return -1;
}

__int32 DevuelveDirecto(char operando[]){
    char *ope;
    ope = &operando[1];
    ope[strlen(ope)-1]='\0';
    if(ope[0] >= '0' && ope[0] <= '9'){
         return (atoi(ope));
    }else{
        switch(ope[0]){ //Con esto vamos a devolver el valor en decimal.
            case '#':
                ope = &ope[1];
                return atoi(ope);
                break;
            case 39: //Si es una letra.
                return (int) ope[1];
                break;
            case '@':
                ope = &ope[1];
                return strtoul(ope, NULL, 8);
                break;
            case '%':
                ope = &ope[1];
                return strtoul(ope, NULL, 16);
                break;
            default:
                return -1;
        }
    }

}

__int32 DevuelveRegistro(char operando[],TRegistros Registros[]){

    __int32 seccionReg;
    __int32 aux;
    __int32 res;
    int condicion;

    strToUpper(operando);
    res = buscaRegistro(operando,Registros);
    if (res!=-1){ //registros de la primer columna (32 bits)
        return res;
    }else{  //puede ser que sea un registro inexistente o que sean los registros AX,AH,AL etc..
        condicion = ((operando[0] >= 'A' && operando[0] <= 'F') || (operando[0]=='S'));
        if ((condicion) && (operando[2]=='\0')) {
            switch (operando[1]) {
                case 'X':  seccionReg = 3;  //11 en la seccion de registro
                break;
                case 'L': seccionReg = 1;   //01 en la seccion de registro
                break;
                case 'H': seccionReg = 2;   //10 en la seccion de registro
                break;
                case 'P': seccionReg=0;
                default: return 0XFFF;    //por ejemplo AZ
            }
            aux = 10 + (operando[0] - 'A');
            return (seccionReg<<4) | aux; 
        }else{
            return 0XFFF; //registro inexistente. por ejemplo MM o EZX
        }
    }

}

__int32 tipoOperando(char op[],ListaRotulos LR){
    char opPC = op[0];
    char opUno= op[1];
    int Inmediato= (opPC=='#' || opPC=='@' || opPC=='%' || (opPC >= 48 && opPC <= 57) || opPC==39 ||opPC=='-' || rotuloInmediato(LR,op) ) ;
    int Directo= (opPC == '[');
    int Indirecto= opPC=='[' && ((opUno>='A' && opUno<='F') || (opUno>='a' && opUno<='f')) ;
    
    if (Indirecto){
        return 3;
    } else if(Inmediato){
        return 0;
    } else if(Directo){
        return 2;
    } else {
        return 1; //de registro
    } 
}

__int32 DevuelveIndirecto(char operando[],TRegistros Registros[], ListaRotulos LR){
    int direccion;

     char *ope, reg[4];
     ope = &operando[1];
     ope[strlen(ope)-1]='\0';   
      strToUpper(ope);
     int i=7, k,j,res;
     __int8 aux;
     char simbol[11];

     int doscaracteres=0;


if (ope[0]=='A' || ope[0]=='B' || ope[0]=='C' || ope[0] == 'D' || ope[0] =='E'|| ope[0] =='F')
{
    if (ope[0]=='A' && ope[1]=='C'){
        strcpy(reg,"AC ");
        doscaracteres=1;
    } else if(ope[0]=='B' && ope[1]=='P'){
        strcpy(reg,"BP ");
        doscaracteres=1;
    }
    else if (ope[0]=='E' && (ope[1]=='A' || ope[1]=='B' || ope[1]=='C' || ope[1] == 'D' || ope[1] =='E'|| ope[1] =='F')){
        reg[0]=ope[0];
        reg[1]=ope[1];
        reg[2]=ope[2];
        reg[3]='\0';
    } else{
        reg[0]='E';
        reg[1]=ope[0];
        reg[2]=ope[1];
        reg[3]='\0';
        doscaracteres=1;
    }
    
} 

printf("Reg: %s \n", reg);

/* for (i=0;i<3;i++){
    if (ope[0]=='A')


    reg[i]=ope[i];
} */

while (i<=15 && strcmp(reg,Registros[i].nombre)!=0) {
  //  printf("Elemento %d del registro: %s \n", i, Registros[i].nombre);
    i++;
}
    

if (i>15)
    printf("Registro inexistente");
else{            
    if (!doscaracteres && ope[3]=='+' || ope[3]=='-'){     //hay offset
      if (ope[4]>='0' && ope[4]<='9'){  //si es un digito
            printf("Offset digito \n");
            k=4; 
            j=0;
            while (ope[k]!='\0'){
                simbol[j++]=ope[k++];
            }
            simbol[j]='\0';
            aux=atoi(simbol);
      } else{                 
            k=4;
            j=0;
            while(ope[k]!='\0')               
                simbol[j++]=ope[k++];      
            simbol[j]='\0';

            ListaRotulos auxRotulos = LR;
            while (auxRotulos!=NULL && strcmp(simbol,auxRotulos->rotulo)!=0)
                auxRotulos=auxRotulos->sig;

            if (auxRotulos!=NULL)
                aux = auxRotulos->linea;    
            else{
                printf("ERROR: Simbolo %s inexistente. Deteniendo traduccion \n", simbol);    
                exit(0);
            }
    
      }
      res = aux << 4 | (i & 0xF);
    } else if (doscaracteres && ope[2]=='+' || ope[2]=='-'){
        if (ope[3]>='0' && ope[3]<='9'){  //si es un digito
            printf("Offset digito 2 caracteres\n");
            k=3; 
            j=0;
            while (ope[k]!='\0'){
                simbol[j++]=ope[k++];
            }
            simbol[j]='\0';
            aux=atoi(simbol);
      } else{                 
            k=3;
            j=0;
            while(ope[k]!='\0')               
                simbol[j++]=ope[k++];      
            simbol[j]='\0';

            ListaRotulos auxRotulos = LR;
            while (auxRotulos!=NULL && strcmp(simbol,auxRotulos->rotulo)!=0)
                auxRotulos=auxRotulos->sig;

            if (auxRotulos!=NULL)
                aux = auxRotulos->linea;    
            else{
                printf("ERROR: Simbolo %s inexistente. Deteniendo traduccion \n", simbol);    
                exit(0);
            }
      }
      res = aux << 4 | (i & 0xF);
    }  else  //no hay offset
        res = i & 0xF;      
}
return (res & 0xFFF);
}


__int32 transformaOperando(char operando[],int tipoOperando,TRegistros Registros[],ListaRotulos LR){

    if (tipoOperando==3){
        return DevuelveIndirecto(operando,Registros,LR);
    } else if(tipoOperando==0){
        return DevuelveInmediato(operando,LR);
    } else if(tipoOperando==2){
        return DevuelveDirecto(operando); 
    } else if (tipoOperando==1){
        return DevuelveRegistro(operando,Registros); 
     } else
        return -1; //Si no es de ningun tipo te tiene que tirar un error
}

int rotuloInmediato(ListaRotulos LR, char operando[]){
    while (LR!=NULL && strcmp(operando,LR->rotulo)!=0)
        LR = LR->sig;
    if (LR!=NULL && strcmp(operando,LR->rotulo)==0)  //si lo encontré
        return 1;
    else
        return 0;
}

  

__int32 DevuelveConstantValue(char operando[]){  
        char *ope, *simbol;
        hexa_u_octal=0;
        
        if(operando[0]== '-' || (operando[0] >= '0' && operando[0] <= '9')) {
            return atoi(operando);     
        }else{
                switch(operando[0]){ //Con esto vamos a devolver el valor en decimal.
                    case '#':
                        ope = &operando[1];
                        return atoi(ope);
                        break;
                    case 39: //Si es una letra.
                        return (int) operando[1];
                        break;
                    case '@':
                    hexa_u_octal=1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 8);
                        break;
                    case '%':
                        hexa_u_octal=1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 16);
                        break;
                    default:
                        return -1;  // Utilizamos -1 para decir que es un error de sintaxis 
                }
            //}
        }
    }