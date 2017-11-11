%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define YYSTYPE node_star

#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

typedef struct node{
	char *id;
	int soncnt, lineno;
	struct node** son;
} node;

int lineno = 1;

typedef struct node* node_star;

node_star _newnode(char *name, int n, ...)
{
	node* ret = (node*) malloc(sizeof(node));
	ret->id = (char*) malloc((strlen(name) + 5) * (sizeof(char)));
	strcpy(ret->id, name);
	va_list vl;
	va_start(vl, n);
	ret->soncnt = n;
	ret->lineno = lineno;
	int soncnt = n;
	if (soncnt)
	{
		ret->son = (node**) malloc((sizeof(node*)) * soncnt);
		for (int i = 0; i < n; i++)
		{
			ret->son[i] = va_arg(vl, node*);
			if (ret->son[i])
				if (ret->son[i]->lineno < ret->lineno)
					ret->lineno = ret->son[i]->lineno;
		}
	} else {
	}
	va_end(vl);
	return ret;
}

#define nnewnode(_000) _newnode(_000, 0)
#define newnode(_000, ...) _newnode(_000, PP_NARG(__VA_ARGS__), ##__VA_ARGS__)

void travel(node_star rt, int lvl)
{
	printf("%3d)", rt->lineno);
	for (int i = 0; i < lvl; i++) printf("  ");
	printf("%s\n", rt->id);
	for (int i = 0; i < rt->soncnt; i++)
		if (rt->son[i])
			travel(rt->son[i], lvl + 1);
}

%}

%token INT OCTINT HEXINT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%left LP RP LB RB DOT

%%

Program : ExtDefList {$$ = newnode("Program", $1);travel($$, 0);}
        ;
ExtDefList : ExtDef ExtDefList {$$ = newnode("ExtDefList", $1, $2);}
           | /* empty */ {$$ = NULL;}
		   ;
ExtDef : Specifier ExtDecList SEMI {$$ = newnode("ExtDef", $1, $2, $2);}
       | Specifier SEMI {$$ = newnode("ExtDef", $1, $2);}
	   | Specifier FunDec CompSt {$$ = newnode("ExtDef", $1, $2, $3);}
	   ;
ExtDecList : VarDec {$$ = newnode("ExtDecList", $1);}
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
         | /* empty */ {$$ = NULL;}
         ;
Stmt : Exp SEMI {$$ = newnode("Stmt", $1, $2);}
     | CompSt {$$ = newnode("Stmt", $1);}
	 | RETURN Exp Stmt {$$ = newnode("Stmt", $1, $2, $3);}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = newnode("Stmt", $1, $2, $3, $4, $5);}
	 | IF LP Exp RP Stmt ELSE Stmt {$$ = newnode("Stmt", $1, $2, $3, $4, $5, $6, $7);}
	 | WHILE LP Exp RP Stmt {$$ = newnode("Stmt", $1, $2, $3, $4, $5);}
	 ;
DefList : Def DefList {$$ = newnode("DefList", $1, $2);}
        | /* empty */ {$$ = NULL;}
		;
Def : Specifier DecList SEMI {$$ = newnode("Def", $1, $2, $3);}
    ;
DecList : Dec {$$ = newnode("DecList", $1);}
        | Dec COMMA DecList {$$ = newnode("DecList", $1, $2, $3);}
		;
Dec : VarDec {$$ = newnode("Dec", $1);}
    | VarDec ASSIGNOP Exp {$$ = newnode("VarDec", $1, $2, $3);}
	;
Exp : Exp ASSIGNOP Exp {$$ = newnode("Exp", $1, $2, $3);}
    | Exp AND Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp OR Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp RELOP Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp PLUS Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp MINUS Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp STAR Exp {$$ = newnode("Exp", $1, $2, $3);}
	| Exp DIV Exp {$$ = newnode("Exp", $1, $2, $3);}
	| LP Exp RP {$$ = newnode("Exp", $1, $2, $3);}
	| MINUS Exp %prec STAR {$$ = newnode("Exp", $1, $2);}
	| NOT Exp {$$ = newnode("Exp", $1, $2);}
	| ID LP Args RP {$$ = newnode("Exp", $1, $2, $3, $4);}
	| ID LP RP {$$ = newnode("Exp", $1, $2, $3);}
	| Exp LB Exp RB {$$ = newnode("Exp", $1, $2, $3, $4);}
	| Exp DOT ID {$$ = newnode("Exp", $1, $2, $3);}
	| ID {$$ = newnode("Exp", $1);}
	| INT {$$ = newnode("Exp", $1);}
	| OCTINT {$$ = newnode("Exp", $1);}
	| HEXINT {$$ = newnode("Exp", $1);}
	| FLOAT {$$ = newnode("Exp", $1);}
	| error ';' {yyerror("Error when parsing Exp on line %d\n", lineno);yyerrok;}
	;
Args : Exp COMMA Args {$$ = newnode("Args", $1, $2, $3);}
     | Exp {$$ = newnode("Args", $1);}
	 ;
%%
#include "lex.yy.c"
