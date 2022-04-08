#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"


typedef struct TRegistros{
    __int32 ValorRegistro;
    char nombre[3];
} TRegistros;


typedef struct {
    char* mnemonico;
    __int32 cod;
} elementosMnemonicos;

typedef struct nodo {
    char* rotulo;
    int linea;
    struct nodo *sig;
}   nodo;

typedef struct nodo* ListaRotulos;

//------------------------------------------TRADUCTOR MV----------------------------------------------------------

void cargaMnemonicos(elementosMnemonicos mnemonicos[]);
void ingresarRotulo(ListaRotulos *LR, char* rotulo, int lineaRotulo);
__int32 recorreMnemonicos(elementosMnemonicos mnemonicos[],char* mnemonico);
char* strToUpper(char* palabra);

int main(int arg, char *args[])
{

    elementosMnemonicos mnemonicos[25];
    ListaRotulos LR = NULL;
    char instruccionAss[256];
    __int32 instruccionBin=0x0;
    FILE *archI;
    FILE *archO;
    archI=fopen(args[1],"rt");  //se abre el archivo de entrada para leer el programa assembler
    archO = fopen(args[2],"wb"); //se abre el archivo de salida de la traduccion para escritura en binario

    char **parsed;
    TRegistros Registros[16];

    if(archI!=NULL)     //si el archivo de entrada no existe o se genera algun error no hace nada
    {
        while (!feof(archI))
        {
             fgets(instruccionAss,256,archI);  //lee la linea correspondiente del archivo asm
             parsed = parseline(instruccionAss);


           /*  printf("    LABEL: %s\n", parsed[0] ? parsed[0] : "");
             printf(" MNEMONIC: %s\n", parsed[1] ? parsed[1] : "");
             printf("OPERAND 1: %s\n", parsed[2] ? parsed[2] : "");
             printf("OPERAND 2: %s\n", parsed[3] ? parsed[3] : "");
             printf("  COMMENT: %s\n", parsed[4] ? parsed[4] : ""); */
             freeline(parsed);
             printf("\n \n ---------------------------------------------------------------------------------------- \n \n");
        }
    }
    fclose(archI);

    printf("Hola pa");

    return 0;
}

__int32 DevuelveInmediato(char operando[]){
    char *ope;

    int es_hexa_u_octal=0;
    if(operando[0]== '-' || (operando[0] >= '0' && operando[0] <= '9'))
        return atoi(operando);
    else{
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

void InicializaHeader(__int32 Header[]){
    Header[0] = 0x4D563231;
    Header[1] = 1024; //Corresponde al DS
    Header[2] = 1024; //Corresponde al SS
    Header[3] = 1024; //Corresponde al ES
    Header[4] = 0; //Corresponde al CS
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
    strcpy(Registros[10].nombre,"AX"); Registros[10].ValorRegistro=0;
    strcpy(Registros[11].nombre,"BX"); Registros[11].ValorRegistro=0;
    strcpy(Registros[12].nombre,"CX"); Registros[12].ValorRegistro=0;
    strcpy(Registros[13].nombre,"DX"); Registros[13].ValorRegistro=0;
    strcpy(Registros[14].nombre,"EX"); Registros[14].ValorRegistro=0;
    strcpy(Registros[15].nombre,"FX"); Registros[15].ValorRegistro=0;
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
    mnemonicos[10].cod=10;
    strcpy(mnemonicos[10].mnemonico,"OR");
    mnemonicos[11].cod=11;
    strcpy(mnemonicos[11].mnemonico,"XOR");
    mnemonicos[12].cod=12;
    strcpy(mnemonicos[12].mnemonico,"SYS");
    mnemonicos[13].cod=13;
    strcpy(mnemonicos[13].mnemonico,"JMP");
    mnemonicos[14].cod=14;
    strcpy(mnemonicos[14].mnemonico,"JZ");
    mnemonicos[15].cod=15;
    strcpy(mnemonicos[15].mnemonico,"JP");
    mnemonicos[16].cod=16;
    strcpy(mnemonicos[16].mnemonico,"JN");
    mnemonicos[17].cod=17;
    strcpy(mnemonicos[17].mnemonico,"JNZ");
    mnemonicos[18].cod=18;
    strcpy(mnemonicos[18].mnemonico,"JNP");
    mnemonicos[19].cod=19;
    strcpy(mnemonicos[19].mnemonico,"JNN");
    mnemonicos[20].cod=20;
    strcpy(mnemonicos[20].mnemonico,"LDL");
    mnemonicos[21].cod=21;
    strcpy(mnemonicos[21].mnemonico,"LDH");
    mnemonicos[22].cod=22;
    strcpy(mnemonicos[22].mnemonico,"RND");
    mnemonicos[23].cod=23;
    strcpy(mnemonicos[23].mnemonico,"NOT");
    mnemonicos[24].cod=24;
    strcpy(mnemonicos[24].mnemonico,"STOP");
}

void ingresarRotulo(ListaRotulos *LR, char* rotulo, int lineaRotulo)
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
    char *mnemonicoMayuscula;
    int i=0;
    strcpy(mnemonicoMayuscula,strToUpper(mnemonico));

    while (strcmp(mnemonicos[i].mnemonico,mnemonico)!=0 && i<25)    //Busca el mnemonico en la lista de mnemonicos
        i++;
    
    if(strcmp(mnemonicos[i].mnemonico,mnemonico)==0)    //Si encuentra el mnemonico devuelve el codigo para la instruccion
        return mnemonicos[i].cod;
    else 
        return 0xFFFFFFFF;      //Si no lo encuentra es que no existe 
    
}

char* strToUpper(char* palabra){      //Pasa a mayusculas el string que le mandas por parametro

    for(int i=0;i<strlen(palabra);i++)
    {
        palabra[i]=toupper(palabra[i]);
    }
    return palabra;
}





