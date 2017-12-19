%error-verbose
%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "extend.h"
#include "node.c"

extern FILE* yyin;
FILE* line_reader;

FILE* result_writer;

char line_buffer[65536];
int lineno = 0, charno = 1;
int legal = 1;

int yylex();

#include "project1.h"
#include "project2.h"
#include "project2.2.h"

// 遍历，内存漏就漏吧= =

void travel(node_star rt, int lvl)
{
	if (!legal) {
		exit(1);
	}
	//printf("[%3d:%3d]->[%3d:%3d]", rt->start_lineno, rt->start_pos, rt->end_lineno, rt->end_pos - 1);
	for (int i = 0; i < lvl; i++) fprintf(result_writer, "  ");
	fprintf(result_writer, "%s %d %d\n", rt->label, rt->soncnt, rt->start_lineno);
	for (int i = 0; i < rt->soncnt; i++)
		if (rt->son[i])
			travel(rt->son[i], lvl + 1);
		else
        {
			for (int i = 0; i < lvl + 1; i++) fprintf(result_writer, "  ");
			fprintf(result_writer, "Eps 0 0\n");
		}
}

%}

// 定义一堆关键字和它们的优先级

%token INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE CBEGIN CEND

// 这样可以避免else的歧义
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

Program : ExtDefList {
			dollarNode( Program );
			_r->programBody = $1;
			$$ = _r;
		}
		;
ExtDefList : ExtDef ExtDefList {
		    dollarNode( ExtDefList );
			_r->code = $1;
			_r->next = $2;
			$$ = _r;
		}
		   | /* empty */ {$$ = NULL;}
		   ;
ExtDef : Specifier ExtDecList SEMI {
			dollarNode( ExtDef );
			_r->spec = $1;
			_r->dec = $2;
			_r->function = NULL;
			_r->functionBody = NULL;
			$$ = _r;
		}
	   | Specifier SEMI {
			dollarNode( ExtDef );
			_r->spec = $1;
			_r->dec = NULL;
			_r->function = NULL;
			_r->functionBody = NULL;
			$$ = _r;
		}
	   | Specifier FunDec CompSt {
			dollarNode( ExtDef );
			_r->spec = $1;
			_r->dec = NULL;
			_r->function = $2;
			_r->functionBody = $3;
			$$ = _r;
		}
	   ;
ExtDecList : VarDec {
			dollarNode( ExtDecList );
			_r->dec = $1;
			_r->next = NULL;
			$$ = _r;
		}
		   | VarDec COMMA ExtDecList {
			dollarNode( ExtDecList );
			_r->dec = $1;
			_r->next = $3;
			$$ = _r;
		}
		   ;
Specifier : TYPE {
			dollarNode( Specifier );
			_r->typeName = pCast(node, $1)->label;
			_r->structName = NULL;
			$$ = _r;
		}
		  | StructSpecifier {
			dollarNode( Specifier );
			_r->typeName = NULL;
			_r->structName = $1;
			$$ = _r;
		}
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC {
			dollarNode( StructSpecifier );
			_r->typeName = $2 ? (pCast(node, $2)->label) : NULL;
			$$ = _r;
		}
				| STRUCT OptTag LC error RC {$$ = NULL;}
				| STRUCT Tag {
			dollarNode( StructSpecifier );
			_r->typeName = (pCast(node, $2)->label);
			$$ = _r;
		}
				;
OptTag : ID {$$ = $1;}
	   | /* empty */ {$$ = NULL;}
	   ;
Tag : ID {$$ = $1;}
	;
VarDec : ID {
			dollarNode( VarDec );
			_r->varName = pCast(node, $1)->label;
			$$ = _r;
		}
	   | ID VarDimList {
			dollarNode( VarDec );
			_r->varName = pCast(node, $1)->label;
			_r->dim = $2;
			$$ = _r;
		}
	   ;
VarDimList : LB INT RB {
			dollarNode( VarDimList );
			_r->thisDim = atoi(pCast(node, $2)->label);
			_r->next = NULL;
			$$ = _r;
		}
		   | LB INT RB VarDimList {
			dollarNode( VarDimList );
			_r->thisDim = atoi(pCast(node, $2)->label);
			_r->next = $4;
			$$ = _r;
		}
           | LB error RB {raise_line_error(charno - 1, charno, _E_COLOR_ERR);$$ = NULL;}
           ;
FunDec : ID LP VarList RP {
			dollarNode( FunDec );
			_r->name = pCast(node, $1)->label;
			_r->varList = $3;
			$$ = _r;
		}
	   | ID LP RP {
			dollarNode( FunDec );
			_r->name = pCast(node, $1)->label;
			_r->varList = NULL;
			$$ = _r;
		}
	   | ID LP error RP {
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
			$$ = NULL;
		}
	   ;
VarList : ParamDec COMMA VarList {
			dollarNode( VarList );
			_r->thisParam = $1;
			_r->next = $3;
			$$ = _r;
		}
		| ParamDec {
			dollarNode( VarList );
			_r->thisParam = $1;
			_r->next = NULL;
			$$ = _r;
		}
		| ParamDec error {yyerror("<<Error Type B.0>> `,' expected");yyerrok;}
		;
ParamDec : Specifier VarDec {
			dollarNode( ParamDec );
			_r->type = $1;
			_r->name = $2;
			$$ = _r;
		}
		 ;
CompSt : LC DefList StmtList RC {
			dollarNode( CompSt );
			_r->defList = $2;
			_r->stmtList = $3;
			$$ = _r;
		}
	   ;
StmtList : Stmt StmtList {
			dollarNode( StmtList );
			_r->statement = $1;
			_r->next = $2;
			$$ = _r;
		}
		 | /* empty */ {$$ = NULL;}
		 ;
Stmt : Exp SEMI {
			dollarNode( Stmt );
			_r->expression = $1;
			_r->compStatement = NULL;
			_r->isReturn = False;
			_r->isWhile = False;
			_r->ifTrue = NULL;
			_r->ifFalse = NULL;
			$$ = _r;
		}
	 | CompSt {
			dollarNode( Stmt );
			_r->expression = NULL;
			_r->compStatement = $1;
			_r->isReturn = False;
			_r->isWhile = False;
			_r->ifTrue = NULL;
			_r->ifFalse = NULL;
			$$ = _r;
		}
	 | RETURN Exp SEMI {
			dollarNode( Stmt );
			_r->expression = $2;
			_r->compStatement = NULL;
			_r->isReturn = True;
			_r->isWhile = False;
			_r->ifTrue = NULL;
			_r->ifFalse = NULL;
			$$ = _r;
		}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
			dollarNode( Stmt );
			_r->expression = $3;
			_r->compStatement = NULL;
			_r->isReturn = False;
			_r->isWhile = False;
			_r->ifTrue = $5;
			_r->ifFalse = NULL;
			$$ = _r;
		}
	 | IF LP Exp RP Stmt ELSE Stmt {
			dollarNode( Stmt );
			_r->expression = $3;
			_r->compStatement = NULL;
			_r->isReturn = False;
			_r->isWhile = False;
			_r->ifTrue = $5;
			_r->ifFalse = $7;
			$$ = _r;
		}
	 | IF LP error RP {
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
		}
	 | WHILE LP Exp RP Stmt {$$ = newnode("StmtWHILE", $3, $5);}
	 | Exp error {
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, ";");
		}
	 | IF error {
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, "(");
		}
	 | WHILE error {
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, ")");
		}
	 | WHILE LP error RP{
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
		}
	 ;
DefList : Def DefList {
			dollarNode( DefList );
			_r->def = $1;
			_r->next = $2;
			$$ = _r;
		}
		| /* empty */ {$$ = NULL;}
		;
Def : Specifier DecList SEMI {
			dollarNode( Def );
			_r->type = $1;
			_r->decList = $2;
			$$ = _r;
		}
	;
DecList : Dec {
			dollarNode( DecList );
			_r->dec = $1;
			$$ = _r;
		}
		| Dec COMMA DecList {
			dollarNode( DecList );
			_r->dec = $1;
			_r->next = $3;
			$$ = _r;
		}
		;
Dec : VarDec ASSIGNOP Exp {
			dollarNode( Dec );
			_r->var = $1;
			_r->value = $3;
			$$ = _r;
		}
	| VarDec {
			dollarNode( Dec );
			_r->var = $1;
			$$ = _r;
		}
	;
Exp : Exp ASSIGNOP Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_ASSIGN;
			$$ = _r;
		}
	| Exp AND Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_AND;
			$$ = _r;
		}
	| Exp OR Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_OR;
			$$ = _r;
		}
	| Exp RELOP Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			if (isLabel($2, "<")) _r->op = EJQ_OP_RELOP_LT;
			else if (isLabel($2, "<=")) _r->op = EJQ_OP_RELOP_LE;
			else if (isLabel($2, "==")) _r->op = EJQ_OP_RELOP_EQ;
			else if (isLabel($2, ">=")) _r->op = EJQ_OP_RELOP_GE;
			else if (isLabel($2, ">"))  _r->op = EJQ_OP_RELOP_GT;
			else if (isLabel($2, "!=")) _r->op = EJQ_OP_RELOP_NE;
			else {
				fprintf(stderr, "Bug, undefined relop!");
				exit(-1);
			}
			$$ = _r;
		}
	| Exp PLUS Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_PLUS;
			$$ = _r;
		}
	| Exp MINUS Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_MINUS;
			$$ = _r;
		}
	| Exp STAR Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_STAR;
			$$ = _r;
		}
	| Exp DIV Exp {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_DIV;
			$$ = _r;
		}
	| LP Exp RP {
			$$ = $2;/* 括号不要啦 */
		} 
	| MINUS Exp %prec STAR {
			dollarNode( Exp );
			_r->lExp = $2;
			_r->op = EJQ_OP_UNARY_MINUS;
			$$ = _r;
		}
	| NOT Exp {
			dollarNode( Exp );
			_r->lExp = $2;
			_r->op = EJQ_OP_UNARY_NOT;
			$$ = _r;
		}
	| ID LP Args RP {
			dollarNode( Exp );
			_r->funcName = (char*)malloc(strlen(getLabel($1) + 5));
			strcpy(_r->funcName, getLabel($1));
			_r->args = $3;
			_r->isFunc = True;
			$$ = _r;
		}
	| ID LP error RP{
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| ID LP RP {
			dollarNode( Exp );
			_r->funcName = (char*) malloc(strlen(getLabel($1) + 5));
			strcpy(_r->funcName, getLabel($1));
			_r->isFunc = True;
			$$ = _r;
		}
	| Exp LB Exp RB {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->rExp = $3;
			_r->op = EJQ_OP_ARRAY;
			$$ = _r;
		}
	| Exp LB error RB {
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| Exp DOT ID {
			dollarNode( Exp );
			_r->lExp = $1;
			_r->funcName = (char*) malloc(strlen(getLabel($3) + 5));
			strcpy(_r->funcName, getLabel($3));
			$$ = _r;
		}
	| ID {
			dollarNode( Exp );
			_r->funcName = (char*) malloc(strlen(getLabel($1)) + 5);
			strcpy(_r->funcName, getLabel($1));
			$$ = _r;
		}
	| INT {
			dollarNode( Exp );
			_r->isImm8 = EJQ_IMM8_INT;
			_r->intVal = atoi(getLabel($1));
			$$ = _r;
		}
	| FLOAT {
			dollarNode( Exp );
			_r->isImm8 = EJQ_IMM8_FLOAT;
			_r->floatVal = atof(getLabel($1));
			$$ = _r;
		}
	;
Args : Exp COMMA Args {
			dollarNode( Args );
			_r->exp = $1;
			_r->next = $3;
			$$ = _r;
		}
	 | Exp {
			dollarNode( Args );
			_r->exp = $1;
			$$ = _r;
		}
	 ;
%%
#include "lexer.c"

int main(int argc, char **argv)
{
	if (argc >= 2)
	{
		yyin = fopen(argv[1], "r");
		line_reader = fopen(argv[1], "r");
	}
	else
	{
		printf("Fatal: %sNo input file.\033[0m\n", write_color(_E_COLOR_ERR));
		return -1;
	}
	if (argc == 3) {
		result_writer = fopen(argv[2], "w");
	} else {
		result_writer = stdout;
	}
	move_to_next_line();
	yyparse();
}
