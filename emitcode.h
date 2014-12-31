#ifndef EMITCODE_H
#define EMITCODE_H

int ArgList[3];
int ArgNum;
int NumOfParams;
int LabelSeed;
FILE *fp;

void emitAluOp(int op, int reg1, int reg2);
enum {ADD, SUB, MULT, DIV} ALU_OPS;

void emitRelOp(int op, int reg1, int reg2);
enum {EQU, NEQU, LESS, GTR, LTEQU, GTEQU} RELATION_OPS;

void emitMemOp(int op, char *id, int reg);
enum {LOAD, STORE} MEMORY_OPS;

void emitLoadConst(int reg, int value);

void emitDeclaration(int type, char *id);
enum {VAR, FUNC} DECLARATION_TYPE;

void emitCall(char *id, int argList[]);


void emitEpilogue();
void emitPrintReturn();

char *regToString(int reg);
enum {EAX, EBX, ECX, EDX, ESI, EDI} REGISTER_NAME;

int nextFreeRegister();
void releaseOneRegister();
void releaseAllRegister();

#endif