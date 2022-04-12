#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include "string.h"
#include <ctype.h>
#include "parser.h"

//int error;

typedef struct TRegistros{
    __int32 ValorRegistro;
    char nombre[3];
} TRegistros;


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

//------------------------------------------TRADUCTOR MV----------------------------------------------------------
int rotuloInmediato(ListaRotulos LR, char operando[]);
int tipoOperando(char op[5],ListaRotulos LR);
__int32 transformaOperando(char operando[5],int tipoOperando,TRegistros Registros[],ListaRotulos LR);
__int32 DevuelveInmediato(char operando[],ListaRotulos LR);
__int32 DevuelveDirecto(char operando[], TRegistros Registros[]);
__int32 DevuelveRegistro(char operando[],TRegistros Registros[]);
__int32 esRotulo(char ope[],ListaRotulos LR);
__int32 buscaRegistro(char operando[],TRegistros Registros[]);
void InicializaRegistros(TRegistros Registros[]);
void buscaRotulo(ListaRotulos *LR, FILE *archA);
void parseo(FILE *archA,char **parsed);
void Traduccion(char **parsed,__int32 *instruccionBin,elementosMnemonicos mnemonicos[],ListaRotulos LR,TRegistros Registros[]);
void InicializaHeader(__int32 Header[]);
void cargaMnemonicos(elementosMnemonicos mnemonicos[]);
void ingresarRotulo(ListaRotulos *LR, char* rotulo, int lineaRotulo);
__int32 recorreMnemonicos(elementosMnemonicos mnemonicos[],char* mnemonico);
void strToUpper(char palabra[]);

int main(int arg, char *args[])
{
    int i=0;    //checkear cuando hay que sumar el numero de linea y cuando no
    __int32 header[6];
    elementosMnemonicos mnemonicos[25];
    ListaRotulos LR = NULL;
    __int32 instruccionBin=0x0;
    FILE *archI;
    FILE *archO;
    archI=fopen(args[1],"rt");  //se abre el archivo de entrada para leer el programa assembler
    archO = fopen(args[2],"wb"); //se abre el archivo de salida de la traduccion para escritura en binario

    char **parsed;
    TRegistros Registros[16];

    //Inicializamos las estructuras 
    InicializaHeader(header);
    cargaMnemonicos(mnemonicos);
    InicializaRegistros(Registros);

    //-------------------------------------------------------
    Registros[0].ValorRegistro=10;

    if(archI!=NULL)     //si el archivo de entrada no existe o se genera algun error no hace nada
    {
        buscaRotulo(&LR,archI);
        fseek(archI,0,SEEK_SET);
        while (!feof(archI))
        {   
            char instruccionAss[256];
            fgets(instruccionAss,256,archI);  //lee la linea correspondiente del archivo asm
            parsed = parseline(instruccionAss);
          // parseo(archI,parsed);
            int lineaVacia = parsed[0]==NULL && parsed[1]==NULL && parsed[2]==NULL && parsed[3]==NULL;
            if((!lineaVacia)){
                Traduccion(parsed,&instruccionBin,mnemonicos,LR,Registros);
            }  else {
            
            } 


            freeline(parsed);
        }
        
 
        

    }
    fclose(archI);

    return 0;
}

/*void parseo(FILE *archA,char **parsed){

        char instruccionAss[256];
        fgets(instruccionAss,256,archA);  //lee la linea correspondiente del archivo asm
        parsed = parseline(instruccionAss);
           
           /* 
            printf("    LABEL: %s\n", parsed[0] ? parsed[0] : "");
            printf(" MNEMONIC: %s\n", parsed[1] ? parsed[1] : "");
            printf("OPERAND 1: %s\n", parsed[2] ? parsed[2] : "");
            printf("OPERAND 2: %s\n", parsed[3] ? parsed[3] : "");
            printf("  COMMENT: %s\n", parsed[4] ? parsed[4] : ""); */
        //  freeline(parsed);             
           // printf("\n \n ---------------------------------------------------------------------------------------- \n \n");
//}

void InicializaHeader(__int32 Header[]){
    Header[0] = 0x4D562D31;
    Header[1] = 1024; //Corresponde al DS
    Header[2] = 0; 
    Header[3] = 0; 
    Header[4] = 0;
    Header[5] = 0x562E3232;
}

void InicializaRegistros(TRegistros Registros[]){
    strcpy(Registros[0].nombre,"DS"); Registros[0].ValorRegistro=0;
    strcpy(Registros[1].nombre,"SS"); Registros[1].ValorRegistro=0;
    strcpy(Registros[2].nombre,"ES"); Registros[2].ValorRegistro=0;
    strcpy(Registros[3].nombre,"CS"); Registros[3].ValorRegistro=0;
    strcpy(Registros[4].nombre,"HP"); Registros[4].ValorRegistro=0;
    strcpy(Registros[5].nombre,"IP"); Registros[5].ValorRegistro=0;
    strcpy(Registros[6].nombre,"SP"); Registros[6].ValorRegistro=0;
    strcpy(Registros[7].nombre,"BP"); Registros[7].ValorRegistro=0;
    strcpy(Registros[8].nombre,"CC"); Registros[8].ValorRegistro=0;
    strcpy(Registros[9].nombre,"AC"); Registros[9].ValorRegistro=0;
    strcpy(Registros[10].nombre,"EAX"); Registros[10].ValorRegistro=0;
    strcpy(Registros[11].nombre,"EBX"); Registros[11].ValorRegistro=0;
    strcpy(Registros[12].nombre,"ECX"); Registros[12].ValorRegistro=0;
    strcpy(Registros[13].nombre,"EDX"); Registros[13].ValorRegistro=0;
    strcpy(Registros[14].nombre,"EEX"); Registros[14].ValorRegistro=0;
    strcpy(Registros[15].nombre,"EFX"); Registros[15].ValorRegistro=0;
}

void cargaMnemonicos(elementosMnemonicos mnemonicos[])  //funcion que carga los mnemonicos con sus respectivos codigos en un arreglo
{
    mnemonicos[0].cod=0;
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

void ingresarRotulo(ListaRotulos *LR, char rotulo[], int lineaRotulo)
{
    ListaRotulos aux;
    aux = (ListaRotulos) malloc(sizeof(nodo));
    aux->linea=lineaRotulo;
    aux->sig = NULL;
    strcpy(aux->rotulo,rotulo);

    if (*LR!=NULL)          //Ingresamos el rotulo siempre por cabecera
        aux->sig = *LR;
    *LR = aux;
}

__int32 recorreMnemonicos(elementosMnemonicos mnemonicos[],char* mnemonico)
{ 
    int i=0;
    strToUpper(mnemonico);

    while (strcmp(mnemonicos[i].mnemonico,mnemonico)!=0 && i<25){    //Busca el mnemonico en la lista de mnemonicos
        i++;
    }
  //  printf("%d %s %s %X\n",i,mnemonicos[i].mnemonico,mnemonico,mnemonicos[i].cod);
    if(strcmp(mnemonicos[i].mnemonico,mnemonico)==0) {   //Si encuentra el mnemonico devuelve el codigo para la instruccion
    //    printf("%X\n",mnemonicos[i].cod);
        return mnemonicos[i].cod;
    }
    else {
        //error=1; //Si no encuentra el mnemonico es un error y no deberia seguir con los operandos.
        return 0xFFFFFFFF;      //Si no lo encuentra es que no existe 
    } 
    
}

void strToUpper(char palabra[]){      //Pasa a mayusculas el string que le mandas por parametro

    for(int i=0;i<strlen(palabra);i++)
    {
        palabra[i]=toupper(palabra[i]);
    }

}

void buscaRotulo(ListaRotulos *LR, FILE *archA){
    char instruccionAss[256];
    char **parsed;
    int i=0;

    while(!feof(archA)){
        fgets(instruccionAss,256,archA);  //lee la linea correspondiente del archivo asm
        parsed = parseline(instruccionAss);
       // parseo(archA,parsed);
        if(parsed[0]!=NULL){
            ingresarRotulo(LR,parsed[0],i);
        }
        freeline(parsed);
        i++;
    }
}

__int32 esRotulo(char ope[],ListaRotulos LR){
        
    while (LR!=NULL && strcmp(ope,LR->rotulo)!=0)
        LR = LR->sig;
    if (LR!=NULL && strcmp(ope,LR->rotulo)==0)  //si lo encontré
        return LR->linea;
    else
        return -1;
}

void Traduccion(char **parsed,__int32 *instruccionBin,elementosMnemonicos mnemonicos[],ListaRotulos LR,TRegistros Registros[]){
    
    //VERIFICAR EL ERROR DE SINTAXIS CUANDO NO ENCUENTRA UN MNEMONICO
    __int32 mnemonico,op1,op2;
    int tipoOpe;
    //error=0;
  
    mnemonico=recorreMnemonicos(mnemonicos,parsed[1]);
    //printf("Traduccion: %X\n",mnemonico);
    //printf("%s",parsed[2]);
    if(mnemonico!=0XFFFFFFFF){ //si no hay error sigue con la ejecucion de la traduccion normal
        if(mnemonico<=0XB){
           // printf("Dos operandos\n");
            //Bloque de dos operandos
        }else if (mnemonico<=0XFB){
           // printf("Un operando\n");
            char op1String[5];
            strcpy(op1String,parsed[2]);
            //hacer una funcion q determine si es un rotulo -> inmediato, le pongo el valor d la linea donde esta ese rótulo
            tipoOpe = tipoOperando(op1String,LR);
            op1 =transformaOperando(op1String,tipoOpe,Registros,LR);
            printf("Operando= %d\n",op1);
            /*
            //Bloque de 1 operando
        } else {
           // printf("Ningun operando\n");
            //Bloque ningun oeprando
        }
        //instruccionBin = mnemonico +op1 +op2


    } else {

    }
}

    __int32 DevuelveInmediato(char operando[], ListaRotulos LR){
        char *ope;

        int es_hexa_u_octal=0;
        if(operando[0]== '-' || (operando[0] >= '0' && operando[0] <= '9'))
            return atoi(operando);
        else{
            if(operando[0]>=65 && operando[0]<=90){
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
                        es_hexa_u_octal = 1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 8);
                        break;
                    case '%':
                        es_hexa_u_octal = 1;
                        ope = &operando[1];
                        return strtoul(ope, NULL, 16);
                        break;
                    default:
                        return -1;
                }
            }
        }
    }

    //ignorar esto(? xD - atte: pao
   /* __int32 DevuelveDirecto(char operando[]){

        char *ope;
        int es_hexa_u_octal=0;

        if (operando[1] >= '0' && operando[1] == '9')   //si es por ejemplo [11]
            return atoi(operando);
        else{
            switch(operando[1]){
                case '#':   //por ejemplo [#11]
                    ope = &operando[2];
                    return atoi(ope);
                    break;
                case 39: //Si es una letra.
                    return (int) operando[2];
                    break;
                case '@':
                    es_hexa_u_octal = 1;
                    ope = &operando[2];
                    return strtoul(ope, NULL, 8);
                    break;
                case '%':
                    es_hexa_u_octal = 1;
                    ope = &operando[2];
                    return strtoul(ope, NULL, 16);
                    break;
                default:
                    return -1; 
            }
        }
    }*/

__int32 buscaRegistro(char operando[],TRegistros Registros[]){
    int i = 0;

    while (i <= 15 && strcmp(Registros[i].nombre,operando)!=0)
        i++;
    if (strcmp(Registros[i].nombre,operando)==0)
        return i;
    else
        return -1;
}

__int32 DevuelveDirecto(char operando[], TRegistros Registros[]){
    char *ope;
    ope = &operando[1];
    ope[strlen(ope)-1]='\0';
    printf("%s",ope);
    int es_hexa_u_octal=0;
    if(ope[0] >= '0' && ope[0] <= '9'){
         return (atoi(ope) + Registros[0].ValorRegistro);
    }else{
        switch(ope[0]){ //Con esto vamos a devolver el valor en decimal.
            case '#':
                ope = &ope[1];
                return atoi(ope) + Registros[0].ValorRegistro;
                break;
            case 39: //Si es una letra.
                return (int) ope[1] + Registros[0].ValorRegistro;
                break;
            case '@':
                es_hexa_u_octal = 1;
                ope = &ope[1];
                return strtoul(ope, NULL, 8) + Registros[0].ValorRegistro;
                break;
            case '%':
                es_hexa_u_octal = 1;
                ope = &ope[1];
                return strtoul(ope, NULL, 16) + Registros[0].ValorRegistro;
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

    //buscaRegistro 
    res = buscaRegistro(operando,Registros);
    if (res!=-1){ //registros de la primer columna (32 bits)
        return res;
    }else{  //puede ser que sea un registro inexistente o que sean los registros AX,AH,AL etc..
        condicion = (operando[0] >= 'A' && operando[0] <= 'F');
        if ((condicion) && (operando[2]=='\0')) {
            switch (operando[1]) {
                case 'X':  seccionReg = 3;  //11 en la seccion de registro
                break;
                case 'L': seccionReg = 1;   //01 en la seccion de registro
                break;
                case 'H': seccionReg = 2;   //10 en la seccion de registro
                break;
                default: return -1;    //por ejemplo AZ
            }
            aux = 10 + (operando[0] - 'A');
            return (seccionReg<<4) | aux; 
        }else
            return -1; //registro inexistente. por ejemplo MM o EZX
    }
        //printf("letra en decimal: %d \n",aux);
        //printf("seccion: %X\n",seccionReg<<4);
        //printf("devuelve: %X\n",(seccionReg<<4) | aux);
}

int tipoOperando(char op[5],ListaRotulos LR){
    char opPC = op[0];
    int Inmediato= (opPC=='#' || opPC=='@' || opPC=='%' || (opPC >= 48 && opPC <= 57) || opPC==39 ||opPC=='-' || rotuloInmediato(LR,op) ) ;
    int Directo= (opPC == '[');
    
    //printf("primer char: %c\n",opPC);

    if(Inmediato){
        return 0;
    } else if(Directo){
        return 2;
    } else {
        return 1; //de registro
    } 

}

__int32 transformaOperando(char operando[5],int tipoOperando,TRegistros Registros[],ListaRotulos LR){

    if(tipoOperando==0){
      //  printf("Inmediato\n");
        return DevuelveInmediato(operando,LR);
    } else if(tipoOperando==2){
        printf("Directo\t");
        //return DevuelveDirecto(operando);
        return DevuelveDirecto(operando,Registros); // Aca iria devuelveDriecto
    } else 
        if (tipoOperando==1){
          //  printf("Registro\n");
            return DevuelveRegistro(operando,Registros); 
        }
        else
            return -1; //Si no es de ningun tipo te tiene que tirar un error
    //return 0;
}

int rotuloInmediato(ListaRotulos LR, char operando[]){
    while (LR!=NULL && strcmp(operando,LR->rotulo)!=0)
        LR = LR->sig;
    if (LR!=NULL && strcmp(operando,LR->rotulo)==0)  //si lo encontré
        return 1;
    else
        return 0;
}

    //  Verificamos parsed[1] mnemonico
    // Tenemos que ver si es de 1/2 o ningun operador
    // Verificamos Operador1 Operador 2 o lo que tenga
    //zayrux

