#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

int main()
{   
    /*
    elementosMnemonicos mnemonicos[25];
    ListaRotulos LR = NULL;
    char instruccionAss[256];
    __int32 instruccionBin=0x0;
    FILE *archI;
    FILE *archO;
    archI=fopen("assembler.asm","rt");  //se abre el archivo de entrada para leer el programa assembler
    archO = fopen("programa.mv1","wb"); //se abre el archivo de salida de la traduccion para escritura en binario

    if(archI!=NULL)     //si el archivo de entrada no existe o se genera algun error no hace nada
    {
        while (!feof(archI))
        {
            fgets(instruccionAss,256,archI);  //lee la linea correspondiente del archivo asm

        }
    }
    fclose(archI);
*/
    printf("Hola pa");

    return 0;
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




