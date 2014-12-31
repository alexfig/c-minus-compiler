#ifndef SYMTABLE_H
#define SYMTABLE_H

#define MAX_NAME_LENGTH 20
#define MAX_SYMBOLS_PER_TABLE 10

struct symbolAttributes{
	int type;
  	int initialized;
  	int references;
	int scope;
	int array;
	int arrSize;
    
/* if symbol is a parameter, parameters is the
 * order of the parameter in the function, e.g. 1st, 2nd...
 * if symbol is a func, it is the # of parameters. else it is 0
 */	
  	int parameters;
  	int localVarStackOffset;
  	int regContainingArrIndex;
};

struct symbolEntry{
	char id[MAX_NAME_LENGTH];
	int type;	/*variable or function*/
	struct symbolAttributes attr;
};

struct symbolTable{
	struct symbolEntry symTab[MAX_SYMBOLS_PER_TABLE];
	struct symbolTable *outerScope;
  	int numOfSym;
  	int numOfLocalVar;
};

struct symbolAttributes parsedSymbolAttributes;

struct symbolTable globalSymTab;
struct symbolTable *CurrentScope;

struct symbolEntry *lookUpSym(char *id);

void insertSym(char *id, struct symbolAttributes attr, int type);
void insertGlobalSym(char *id, struct symbolAttributes attr, int type);

void initializeScope();
void finalizeScope();

void printSym(struct symbolEntry sym);

void resetparsedSymbolAttributes();

int inFunctionBody();

#endif
