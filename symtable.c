#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"



struct symbolTable globalSymTab={.outerScope=NULL, .numOfSym=0,.numOfLocalVar=0};
struct symbolTable *CurrentScope = &globalSymTab;

int ScopeLevel = 0;

struct symbolAttributes parsedSymbolAttributes = {
  .type=0,
  .initialized=0,
  .references=0,
  .scope=0,
  .array=0,
  .arrSize=0,
  .parameters=0,
  .localVarStackOffset=0
};

struct symbolEntry *lookUpSym(char *id){
  struct symbolTable *tableIterator = CurrentScope;
  while(tableIterator != NULL){
    for(int i=0; i < tableIterator->numOfSym; i++){
      if(strcmp(tableIterator->symTab[i].id, id) == 0){
        return &tableIterator->symTab[i];
      }
    }
    tableIterator = tableIterator-> outerScope;
  }
  printf("error - id: %s was never declared\n", id);
  exit(0);
}

void insertSym(char *id, struct symbolAttributes attr, int type){
  struct symbolEntry symbol = {.type = type,
                               .attr = attr
                              };
  strcpy(symbol.id, id);
  if(strcmp(id, "main")==0)
    symbol.attr.references=1;
        
  printf("adding symbol to table\n");
  printSym(symbol);
  printf("\n\n");
    
  struct symbolTable *tableIterator = CurrentScope;
  while(tableIterator != NULL){
    for(int i=0; i < tableIterator->numOfSym; i++){
      if(strcmp(tableIterator->symTab[i].id, id) == 0){
        printf("error - id: %s was previously declared\n", id);
        exit(0);
      }
    }
    tableIterator = tableIterator-> outerScope;
  }
  CurrentScope->symTab[CurrentScope->numOfSym] = symbol;
  CurrentScope->numOfSym++;
}

void insertGlobalSym(char *id, struct symbolAttributes attr, int type){
  struct symbolEntry symbol = {.type = type,
                               .attr = attr
                              };
  strcpy(symbol.id, id);
  if(strcmp(id, "main")==0)
    symbol.attr.references=1;
  
  printf("adding symbol to table\n");
  printSym(symbol);
  printf("\n\n");

  struct symbolTable *tableIterator = &globalSymTab;
  for(int i=0; i < tableIterator->numOfSym; i++){
      if(strcmp(tableIterator->symTab[i].id, id) == 0){
        printf("error - id: %s was previously declared\n", id);
        exit(0);
      }
  }
  tableIterator->symTab[tableIterator->numOfSym] = symbol;
  tableIterator->numOfSym++;
}

void initializeScope(){
  ScopeLevel++;
  printf("Initializing new scope. Scope depth:%i\n", ScopeLevel);
  
  struct symbolTable *newTable = (struct symbolTable*) malloc(sizeof(struct symbolTable));
  newTable->outerScope = CurrentScope;
  newTable->numOfSym = 0;
    //newTable->nextReg = 0;
  CurrentScope = newTable;
}

void finalizeScope(){
  printf("Exiting scope level:%i\nPrinting Table\n\n", ScopeLevel);
  printf("Name\t\tType\t\tAttributes\n");
  
  for(int i=0; i < CurrentScope->numOfSym; i++){
    printSym(CurrentScope->symTab[i]);
    printf("\n");

    if(CurrentScope->symTab[i].attr.references == 0){
      printf("warning - id \"%s\" was not referenced\n\n",
      CurrentScope->symTab[i].id);
    }
  }
  printf("\n");

  if(ScopeLevel){
    struct symbolTable *temp = CurrentScope;
    CurrentScope = CurrentScope->outerScope;
    free(temp);
    temp = NULL;
    ScopeLevel--;
  }
}

enum {VAR, FUNC};
void printSym(struct symbolEntry sym){
  printf("%s\t", sym.id);
  if(strlen(sym.id) <9) printf("\t");
  if(sym.type == VAR)
    printf("Variable\t");
  else
    printf("Function\tParameters:%i\t", sym.attr.parameters);
  
  if(sym.attr.type == 0)
    printf("INT\t");
  else
    printf("VOID\t");

  if(sym.attr.array == 1)
    printf("array, size:%i", sym.attr.arrSize);
}
void resetparsedSymbolAttributes(){
  parsedSymbolAttributes.type=0;
  parsedSymbolAttributes.initialized=0;
  parsedSymbolAttributes.references=0;
  parsedSymbolAttributes.scope=0;
  parsedSymbolAttributes.array=0;
  parsedSymbolAttributes.arrSize=0;
  parsedSymbolAttributes.parameters=0;
  parsedSymbolAttributes.localVarStackOffset=0;
}

int inFunctionBody(){
  if (ScopeLevel == 1)
    return 1;
  return 0;
}