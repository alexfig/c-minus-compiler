#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emitcode.h"
#include "symtable.h"

int LabelSeed = 0;
int NumOfParams = 0;
int ArgList[3];
int ArgNum = 0;

FILE *fp;

int nextFreeReg = 0;
const int MaxNumOfLocalVar = 4;



void emitDeclaration(int type, char *id){
    if(type == FUNC){
        fprintf(fp, "global %s\n", id);
        //fprintf(fp, "\t.type %s, @function\n", id);
        fprintf(fp, "%s:\n", id);
        
        fprintf(fp, ";boilerplate prologue\n");
        fprintf(fp, "push ebp\n");
        fprintf(fp, "mov ebp, esp\n");
        fprintf(fp, "push ebx\n");
        fprintf(fp, "push esi\n");
        fprintf(fp, "push edi\n");
        /*make room for local variables*/
        fprintf(fp, "sub esp, %i\n\n", 4*MaxNumOfLocalVar);
    }
    else if(type == VAR){
        if(lookUpSym(id)->attr.array){
            fpos_t pos;
            fgetpos(fp, &pos);
            fseek(fp, 14, SEEK_SET);
            fprintf(fp, "%s times %i dd 0\n", id,lookUpSym(id)->attr.arrSize);
            fsetpos(fp, &pos);
        }
        else if(CurrentScope == &globalSymTab)
            fprintf(fp, "%s: dd\n", id);
        else
            lookUpSym(id)->attr.localVarStackOffset = CurrentScope->numOfLocalVar++;
    }
    else {printf("invalid input to emitDeclration\n"); exit(0);}
}

void emitAluOp(int op, int reg1, int reg2){
    switch(op){
        case ADD:
            fprintf(fp, "add %s, %s\n",regToString(reg1),regToString(reg2));
            break;
        case SUB:
            fprintf(fp, "sub %s, %s\n",regToString(reg1),regToString(reg2));
            break;
        case MULT:
            fprintf(fp, "imul %s, %s\n",regToString(reg1),regToString(reg2));
            break;
        case DIV:
            fprintf(fp, "idiv %s, %s\n",regToString(reg1),regToString(reg2));
            break;
    }
}

void emitRelOp(int op, int reg1, int reg2){
    fprintf(fp, "cmp %s, %s\n",regToString(reg1),regToString(reg2));
    switch(op){
        case EQU:
            fprintf(fp, "je Equ%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndEqu%i\n", LabelSeed);
            fprintf(fp, "Equ%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndEqu%i:\n", LabelSeed);
            LabelSeed++;
            break;
        case NEQU:
            fprintf(fp, "jne Nequ%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndNequ%i\n", LabelSeed);
            fprintf(fp, "Nequ%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndNequ%i:\n", LabelSeed);
            LabelSeed++;
            break;
        case LESS:
            fprintf(fp, "jl Lt%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndLt%i\n", LabelSeed);
            fprintf(fp, "Lt%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndLt%i:\n", LabelSeed);
            LabelSeed++;
            break;
        case GTR:
            fprintf(fp, "jg Gt%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndGt%i\n", LabelSeed);
            fprintf(fp, "Gt%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndGt%i:\n", LabelSeed);
            LabelSeed++;
            break;
        case LTEQU:
            fprintf(fp, "jle Lte%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndLte%i\n", LabelSeed);
            fprintf(fp, "Lte%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndLte%i:\n", LabelSeed);
            LabelSeed++;
            break;
        case GTEQU:
            fprintf(fp, "jge Gte%i\n", LabelSeed);
            fprintf(fp, "mov %s, 0\n", regToString(reg1));
            fprintf(fp, "jmp EndGte%i\n", LabelSeed);
            fprintf(fp, "Gte%i:\n", LabelSeed);
            fprintf(fp, "mov %s, 1\n", regToString(reg1));
            fprintf(fp, "EndGte%i:\n", LabelSeed);
            LabelSeed++;
            break;
    }
}

void emitMemOp(int op, char *id, int reg){
	struct symbolEntry *sym=lookUpSym(id);


	if(op == LOAD){
        if(!sym->attr.initialized){
            printf("error: variable %s not initialized\n",sym->id);
            exit(0);
        }
        
        if(lookUpSym(id)->attr.array){
            fprintf(fp, "mov %s, %s\n", regToString(nextFreeReg), id);
            fprintf(fp, "mov %s, [%s+4*%s]\n", regToString(reg),
                    regToString(nextFreeReg),regToString(lookUpSym(id)->attr.regContainingArrIndex));
        }
        else if(CurrentScope == &globalSymTab)
            fprintf(fp, "mov %s, [%s]\n", regToString(reg), id);
            
		else if(sym->attr.parameters==0)
            fprintf(fp, "mov %s, [ebp-%i]\n", regToString(reg), 4*(sym->attr.localVarStackOffset+3));

        else
            fprintf(fp, "mov %s, [ebp+%i]\n", regToString(reg), 4*(sym->attr.parameters+1));
    }
    else if(op == STORE){
            sym->attr.initialized = 1;

            if(lookUpSym(id)->attr.array){
                fprintf(fp, "mov %s, %s\n", regToString(nextFreeReg), id);
                fprintf(fp, "mov [%s+4*%s], %s\n", regToString(nextFreeReg),
                        regToString(lookUpSym(id)->attr.regContainingArrIndex),regToString(reg));
            }
            else if(CurrentScope == &globalSymTab)
                fprintf(fp, "mov [%s], %s\n", id, regToString(reg));
            else if(sym->attr.parameters==0)
                fprintf(fp, "mov [ebp-%i], %s\n", 4*(sym->attr.localVarStackOffset+3), regToString(reg));
            else
                fprintf(fp, "mov [ebp+%i], %s\n", 4*(sym->attr.parameters+1), regToString(reg));
    }
    else{printf("invalid input to emitMemOp\n"); exit(0);}
}

void emitLoadConst(int reg, int value){
    fprintf(fp, "mov %s, %i\n",regToString(reg), value);

}

void emitCall(char *id, int argList[]){
    struct symbolEntry *tmp =lookUpSym(id);
    if(tmp->attr.parameters!= NumOfParams){
        printf("error: incorrect number of parameters\n");
        exit(0);
    }
    tmp->attr.references++;

    fprintf(fp, "\n;precall\n");
    fprintf(fp, ";save required resgisters, if any\n");
    for(int i=0; i < nextFreeReg - NumOfParams; i++)
        fprintf(fp, "push %s\n", regToString(i));
        
    fprintf(fp, ";push parameters\n");
    for(int i=NumOfParams-1; i >= 0; i--)
        fprintf(fp, "push %s\n", regToString(argList[i]));
    fprintf(fp, "call %s\n", id);
    
    fprintf(fp, "\n;postcall\n");
    fprintf(fp, ";store return value\n");
    if(nextFreeReg-NumOfParams !=EAX)
        /*registers holding parameters can be freed*/
        fprintf(fp, "mov %s, eax\n", regToString(nextFreeReg-NumOfParams));
        
    fprintf(fp, "\n;clear stack of parameters\n");
        fprintf(fp, "add esp, %i\n", 4*NumOfParams);

    fprintf(fp, "\n;restore used resgisters, if any\n");
    for(int i= nextFreeReg - NumOfParams-1 ; i >=0 ; i--)
        fprintf(fp, "pop %s\n", regToString(i));
        
    fprintf(fp, "\n");

    nextFreeReg -= NumOfParams;
}

void emitEpilogue(){
    fprintf(fp, "\n;boilerplate, epilogue\n");
    fprintf(fp, "pop edi\n");
    fprintf(fp, "pop esi\n");
    fprintf(fp, "pop ebx\n");
    fprintf(fp, "mov esp,ebp\n");
    fprintf(fp, "pop ebp\n");
    fprintf(fp, "ret\n\n");

}

char *regToString(int reg){
    switch(reg){
        case EAX:
            return "eax";
        case EBX:
            return "ebx";
        case ECX:
            return "ecx";
        case EDX:
            return "edx";
        case ESI:
            return "esi";
        case EDI:
            return "edi";
	default:
	   printf("error- no more registers. reg=%i\n", reg);
	   exit(0);
    }
}
void emitPrintReturn(){
	fprintf(fp, "\n;print return value\n");
	fprintf(fp, "push eax\n");
	fprintf(fp, "push ReturnMsg\n");
	fprintf(fp, "call printf\n");
	fprintf(fp, "add esp,4\n");
	fprintf(fp, "pop eax\n");
}

int nextFreeRegister(){
    int tmp = nextFreeReg;
    nextFreeReg++;
    return tmp;
}
void releaseOneRegister(){
    nextFreeReg--;
}
void releaseAllRegister(){
    nextFreeReg=0;
}