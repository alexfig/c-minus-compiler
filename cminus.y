/* C-Minus BNF Grammar */
%{ 
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "emitcode.h"
  #include "symtable.h"

  extern int yylineno;
  int yylex();
  void yyerror(const char *s);

  extern struct symbolAttributes parsedSymbolAttributes;

  extern struct symbolTable globalSymTab;
  extern struct symbolTable *CurrentScope;

  extern int LabelSeed;
  extern int NumOfParams;
  extern int ArgList[3];
  extern int ArgNum;

  extern FILE *fp;
  int TextSection = 0; 
%}

%union {
   	int n;
  	char s[15];
}

%token ELSE
%token IF
%token INT
%token RETURN
%token VOID
%token WHILE

%token ID
%token NUM

%token LTE
%token GTE
%token EQUAL
%token NOTEQUAL

%type<n>    NUM call factor term expression type_specifier simple_expression additive_expression addop mulop relop iteration_stmt selection_stmt ifsubroutine whilesubroutine

%type<s> ID var var_declaration

%nonassoc IFX

%nonassoc ELSE
%%

program : InitDataSection declaration_list {finalizeScope();};
                                                                  
InitDataSection : /*empty*/   {fprintf(fp, "SECTION .data\n                                                \n");
						                   fprintf(fp, "ReturnMsg: db \"Return Value:%%i\",10,0\n");
                              }

declaration_list : declaration_list declaration     
                 | declaration          
                 ;

declaration  : var_declaration 
	           | fun_declaration
             ;

var_declaration : type_specifier ID ';'	{insertSym($2, parsedSymbolAttributes, VAR);
                                         strcpy($$, $2);
                                         resetparsedSymbolAttributes();
                                        }
                | type_specifier ID '[' NUM ']' ';' {parsedSymbolAttributes.array = 1;
                                                     parsedSymbolAttributes.arrSize = $4;
                                                     insertSym($2, parsedSymbolAttributes, VAR);
                                                     strcpy($$, $2);
                                                     resetparsedSymbolAttributes();
                                                    }
                ;

type_specifier : INT	  {$$ = 0; parsedSymbolAttributes.type = 0;}
               | VOID   {$$ = 1; parsedSymbolAttributes.type = 1;}
	             ;

fun_declaration : type_specifier ID '('     {if(!TextSection++) fprintf(fp, "\nSECTION .text\nextern printf\n");
                                             emitDeclaration(FUNC, $2);
                                             /*funcBody = 1;*/
                                             initializeScope();
                                            }
                  params ')'                {parsedSymbolAttributes.type = $1;
                                             parsedSymbolAttributes.parameters = NumOfParams;
                                             insertGlobalSym($2, parsedSymbolAttributes, FUNC);
											                       resetparsedSymbolAttributes();
                                             NumOfParams=0;
											                      }
				  compound_stmt  			              {finalizeScope();
                                             if($1 == 1)
                                              	emitEpilogue();      
                                             releaseAllRegister();
                                            }
                ;

params : param_list | VOID ;

param_list : param_list ',' param {NumOfParams++; }
           | param 		            {NumOfParams++; }
	         ;

param : type_specifier ID           {parsedSymbolAttributes.parameters = NumOfParams + 1;
                                     parsedSymbolAttributes.initialized = 1;
                                     insertSym($2, parsedSymbolAttributes, VAR);
                                     resetparsedSymbolAttributes();
                                    }
      | type_specifier ID '[' ']'   {parsedSymbolAttributes.parameters = NumOfParams + 1;
                                     parsedSymbolAttributes.array = 1;
                                     parsedSymbolAttributes.initialized = 1;
                                     insertSym($2, parsedSymbolAttributes, VAR);
                                     resetparsedSymbolAttributes();
                                    }
      ;

                    /* A function's scope is initilized before its parameters are
                     * read. Here we check to see if the compound_stmt is the
                     * function's body or a nested scope, since we already initalized
                     * function body scope.
                     */

compound_stmt : '{'                                     {if(!inFunctionBody())
                                                            initializeScope();
                                                        }
                 local_declarations statement_list '}'  {if(!inFunctionBody())
                                                            finalizeScope();}
              ;

local_declarations : local_declarations var_declaration {emitDeclaration(VAR, $2);}
                   | /* empty */ ;

statement_list : statement_list statement
               | /* empty */ ;

statement : expression_stmt
          | compound_stmt
          | selection_stmt
          | iteration_stmt
          | return_stmt ;

expression_stmt : expression ';'
                | ';' 
				;

selection_stmt : ifsubroutine  statement        {fprintf(fp, "EndIf%i:\n", $1);}
               | ifsubroutine  statement ELSE	  {fprintf(fp, "jmp EndIfElse%i\n", $1);
                              					         fprintf(fp, "EndIf%i:\n", $1);
                      							            }
				         statement					 	          {fprintf(fp, "EndIfElse%i:\n", $1);}
               ;

ifsubroutine : IF  '(' expression ')'  {$$ = LabelSeed; LabelSeed++;
                     					          fprintf(fp, "cmp %s, 1\n", regToString($3));
                                        fprintf(fp, "jne EndIf%i\n", $$);
                                        releaseOneRegister();
                                       }
		   ;

iteration_stmt : whilesubroutine

                '(' expression ')' {fprintf(fp, "cmp %s, 1\n", regToString($<n>3));
                                     fprintf(fp, "jne EndWhile%i\n", $1);
                                     releaseOneRegister();
                                    }
                 statement          {fprintf(fp, "jmp While%i\n", $1);
                                     fprintf(fp, "EndWhile%i:\n", $1);
                                    }
                ;
whilesubroutine : WHILE    {$$ = LabelSeed; LabelSeed++; 
                            fprintf(fp, "While%i:\n", $$);
                           }

return_stmt : RETURN ';'              {emitEpilogue();}

            | RETURN expression ';'   {if($2 == EAX){
										                    emitPrintReturn();
                                        emitEpilogue();
									                     }
                                       else{
                                        fprintf(fp, "mov eax, %s\n", regToString($2));
										                    emitPrintReturn();
                                        emitEpilogue();
                                       }
                                       releaseOneRegister();
                                      }

            ;

expression : var '=' expression     {$$=9;
                                     emitMemOp(STORE,$1,$3);
                                     releaseOneRegister();
                                    }
           | simple_expression      {$$ = $1;}
           ;

var : ID                    {lookUpSym($1)->attr.references++; strcpy($$, $1);}

    | ID '[' expression ']' {struct symbolEntry *tmp = lookUpSym($1);
                             if(!tmp->attr.array)
                             printf("error - %s is not an array", tmp->id);
                             tmp->attr.references++;
                             tmp->attr.regContainingArrIndex = $3;
                             strcpy($$, $1);
                            }
    ;

simple_expression : additive_expression relop additive_expression  {$$=$1;
                                                                    emitRelOp($2,$1,$3);
                                                                    releaseOneRegister();
                                                                   }
                  | additive_expression                            {$$ = $1;
                                                                   }
                  ;

relop : LTE {$$=LTEQU;}| '<'{$$=LESS;} | '>' {$$=GTR;}| GTE{$$=GTEQU;} | EQUAL{$$=EQU;} | NOTEQUAL {$$=NEQU;};

additive_expression : additive_expression addop term    {$$ = $1;
                                                         emitAluOp($2,$1,$3);
                                                         releaseOneRegister();
                                                        }
                    | term                              {$$ = $1;}
                    ;

addop : '+' {$$ = ADD;}
      | '-' {$$ = SUB;}
      ;

term : term mulop factor    {$$ = $1;
                             emitAluOp($2,$1,$3);
                             releaseOneRegister();
                            }
     | factor               {$$ = $1;}
     ;

mulop : '*' {$$ = MULT;}
      | '/' {$$ = DIV;}
      ;

factor : '(' expression ')' {$$ = $2;}
       | var                {
                             $$ = nextFreeRegister();
                             emitMemOp(LOAD,$1,$$);
                            }

       | call               {$$ = $1;}
       | NUM                {$$=nextFreeRegister();
                             emitLoadConst($$, $1);
                            }
       ;

call : ID '(' args ')'  {
                         emitCall($1, ArgList);
                         $$=nextFreeRegister();
                         NumOfParams=0;
                        }
     ;

args : arg_list | /* empty */ ;

arg_list : arg_list ',' expression {ArgList[NumOfParams++] = $3;}
         | expression              {ArgList[NumOfParams++] = $1;}
         ;

%%
int main(){	
	fp = fopen ("output.asm", "w");
	if(!yyparse())
		printf("\nParsing complete\n");
	else
		printf("\nParsing failed\n");
	fclose(fp);
    	return 0;
}




void yyerror (char const *s)
{
  fprintf (stderr, "%s, line:%i\n", s, yylineno);
}


