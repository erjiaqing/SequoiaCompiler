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
			_r->typeName = $1->label;
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
			_r->typeName = $2 ? (pCast(node_star, $2)->label) : NULL;
			$$ = _r;
		}
				| STRUCT OptTag LC error RC {$$ = NULL;}
				| STRUCT Tag {
			dollarNode( StructSpecifier );
			_r->typeName = (pCast(node_star, $2)->label);
			$$ = _r;
		}
				;
OptTag : ID {$$ = $1;}
	   | /* empty */ {$$ = NULL;}
	   ;
Tag : ID {$$ = $1}
	;
VarDec : ID {$$ = $1}
	   | ID VarDimList {
			dollarNode( VarDec );
			_r->varName = pCast(node_star, $1)->label;
			_r->dim = $2;
			$$ = _r;
		}
	   ;
VarDimList : LB INT RB {
			dollarNode( VarDimList );
			_r->thisDim = atoi(pCast(node_star, $2)->label);
			_r->next = NULL;
			$$ = _r;
		}
		   | LB INT RB VarDimList {
			dollarNode( VarDimList );
			_r->thisDim = atoi(pCast(node_star, $2)->label);
			_r->next = $4;
			$$ = _r;
		}
           | LB error RB {raise_line_error(charno - 1, charno, _E_COLOR_ERR);$$ = NULL;}
           ;
FunDec : ID LP VarList RP {
			dollarNode( FunDec );
			_r->name = pCast(node_star, $1)->label;
			_r->varList = $3;
			$$ = _r;
		}
	   | ID LP RP {
			dollarNode( FunDec );
			_r->name = pCast(node_star, $1)->label;
			_r->varList = NULL;
			$$ = _r;
		}
	   | ID LP error RP {
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
			$$ = NULL;
		}
	   ;
VarList : ParamDec COMMA VarList {$$ = newnode("VarList", $1, $3);}
		| ParamDec {$$ = newnode("VarList", $1);}
		| ParamDec error {yyerror("<<Error Type B.0>> `,' expected");yyerrok;}
		;
ParamDec : Specifier VarDec {$$ = newnode("ParamDec", $1, $2);}
		 ;
CompSt : LC DefList StmtList RC {$$ = newnode("CompSt", $2, $3);}
	   ;
StmtList : Stmt StmtList {$$ = newnode("StmtList", $1, $2);}
		 | /* empty */ {$$ = NULL;}
		 ;
Stmt : Exp SEMI {$$ = newnode("StmtEXP", $1);}
	 | CompSt {$$ = newnode("StmtCOMP", $1);}
	 | RETURN Exp SEMI {$$ = newnode("StmtRET", $1, $2);}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = newnode("StmtIF", $3, $5);}
	 | IF LP Exp RP Stmt ELSE Stmt {$$ = newnode("StmtIFEL", $3, $5, $7);}
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
DefList : Def DefList {$$ = newnode("DefList", $1, $2);}
		| /* empty */ {$$ = NULL;}
		;
Def : Specifier DecList SEMI {$$ = newnode("Def", $1, $2);}
	;
DecList : Dec {$$ = newnode("DecList", $1);}
		| Dec COMMA DecList {$$ = newnode("DecList", $1, $3);}
		;
Dec : VarDec ASSIGNOP Exp {$$ = newnode("VarDecAssign", $1, $2, $3);}
	| VarDec {$$ = newnode("VarDec", $1);}
	;
Exp : Exp ASSIGNOP Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp AND Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp OR Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp RELOP Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp PLUS Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp MINUS Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp STAR Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| Exp DIV Exp {$$ = newnode("Exp2", $1, $2, $3);}
	| LP Exp RP {$$ = $2;/* 括号不要啦 */} 
	| MINUS Exp %prec STAR {$$ = newnode("Exp1", $1, $2);}
	| NOT Exp {$$ = newnode("Exp1", $1, $2);}
	| ID LP Args RP {$$ = newnode("ExpFUNC", $1, $3);}
	| ID LP error RP{
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| ID LP RP {$$ = newnode("ExpFUNC", $1);}
	| Exp LB Exp RB {$$ = newnode("ExpARRAY", $1, $3);}
	| Exp LB error RB {
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| Exp DOT ID {$$ = newnode("ExpSTRUCT", $1, $3);}
	| ID {$$ = newnode("ExpID", $1);}
	| INT {$$ = newnode("ExpINT", $1);}
	| FLOAT {$$ = newnode("ExpFLOAT", $1);}
	;
Args : Exp COMMA Args {$$ = newnode("Args", $1, $3);}
	 | Exp {$$ = newnode("Args", $1);}
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
