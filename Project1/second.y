%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define YYSTYPE (node*)

struct node{
	char *id;
	int soncnt;
	node** son;
};

node* newnode(char *name, ...)
{
	node* ret = (node*) malloc(sizeof node);
	ret->id = (char*) malloc((strlen(name) + 1) * (sizeof char));
	strcpy(ret->id, name);
	va_list vl;
	va_start(vl, n);
	ret->soncnt = n;
	int soncnt = n;
	if (soncnt)
	{
		ret->son = (node**) malloc((sizeof(node*)) * soncnt);
		for (int i = 0; i < n; i++) ret->son[0] = va_arg(vl, node*);
	}
	va_end(vl);
	return ret;
}

void travel(node* rt, int lvl)
{
	for (int i = 0; i < lvl; i++) printf("  ");
	printf("%s\n", rt->id);
	for (int i = 0; i < rt->soncnt; i++)
		if (rt->son[i])
			travel(rt->son[i], lvl + 1);
}

%}

%token INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left LP RP LB RB DOT
%right NOT
%left STAR DIV
%left PLUS MINUS
%left RELOP
%left AND
%left OR
%right ASSIGNOP

%%

Program : ExtDefList {$$ = newnode("Program", $1);travel($$);}
        ;
ExtDefList : ExtDef ExtDefList {$$ = newnode("ExtDefList", $1, $2);}
           | /* empty */ {$$ = NULL;}
		   ;
ExtDef : Specifier ExtDecList SEMI {$$ = newnode("ExtDef", $1, $2, $2);}
       | Specifier SEMI {$$ = newnode("ExtDef", $1, $2);}
	   | Specifier FunDec CompSt {$$ = newnode("ExtDef", $1, $2, $3);}
	   ;
ExtDecList : VarDec {$$ = newnode("ExtDecList", $1)}
           | VarDec COMMA ExtDecList {$$ = newnode("ExtDecList", $1, $2, $3);}
		   ;
Specifier : TYPE {$$ = newnode("Specifier", $1);}
          | StructSpecifier {$$ = newnode("StructSpecifier", $1);}
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = newnode("StructSpecifier", $1, $2, $3, $4, $5);}
                | STRUCT Tag {$$ = newnode("StructSpecifier", $1, $2);}
				;
OptTag : ID {$$ = newnode("OptTag", $1);}
       | /* empty */ {$$ = NULL;}
	   ;
Tag : ID {$$ = newnode("Tag", $1);}
    ;
VarDec : ID {$$ = newnode("VarDec", $1);}
       | VarDec LB INT RB {$$ = newnode("VarDec", $1, $2, $3, $4);}
	   ;
FunDec : ID LP VarList RP {$$ = newnode("FunDec", $1, $2, $3, $4);}
       | ID LP RP {$$ = newnode("FunDec", $1, $2, $3);}
	   ;
VarList : ParamDec COMMA VarList {$$ = newnode("VarList", $1, $2, $3);}
        | ParamDec {$$ = newnode("VarList", $1);}
		;
ParamDec : Specifier VarDec {$$ = newnode("ParamDec", $1, $2);}
         ;
CompSt : LC DefList StmtList RC {$$ = newnode("CompSt", $1, $2, $3, $4);}
       ;
StmtList : Stmt StmtList {$$ = newnode("StmtList", $1, $2);}
         | /* empty */ {$$ = newnode("StmtList");}
         ;
Stmt : Exp SEMI
     | CompSt
	 | RETURN Exp Stmt
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
	 | IF LP Exp RP Stmt ELSE Stmt
	 | WHILE LP Exp RP Stmt
	 ;
DefList : Def DefList
        | /* empty */
		;
Def : Specifier DecList SEMI
    ;
DecList : Dec
        | Dec COMMA DecList
		;
Dec : VarDec
    | VarDec ASSIGNOP Exp
	;
Exp : Exp ASSIGNOP Exp
    | Exp AND Exp
	| Exp OR Exp
	| Exp RELOP Exp
	| Exp PLUS Exp
	| Exp MINUS Exp
	| Exp STAR Exp
	| Exp DIV Exp
	| LP Exp RP
	| MINUS Exp %prec STAR
	| NOT Exp
	| ID LP Args RP
	| ID LP RP
	| Exp LB Exp RB
	| Exp DOT ID
	| ID
	| INT
	| FLOAT
	;
Args : Exp COMMA Args
     | Exp
	 ;
%%
#include "lex.yy.c"
