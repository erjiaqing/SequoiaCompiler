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

Program : ExtDefList {$$ = newnode("Program", $1);travel($$, 0);}
		;
ExtDefList : ExtDef ExtDefList {$$ = newnode("ExtDefList", $1, $2);}
		   | /* empty */ {$$ = NULL;}
		   ;
ExtDef : Specifier ExtDecList SEMI {$$ = newnode("ExtDef", $1, $2);}
	   | Specifier SEMI {$$ = newnode("ExtDef", $1);}
	   | Specifier FunDec CompSt {$$ = newnode("ExtDef", $1, $2, $3);}
	   ;
ExtDecList : VarDec {$$ = newnode("ExtDecList", $1);}
		   | VarDec COMMA ExtDecList {$$ = newnode("ExtDecList", $1, $3);}
		   ;
Specifier : TYPE {$$ = newnode("Specifier", $1);}
		  | StructSpecifier {$$ = newnode("StructSpecifier", $1);}
		  ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = newnode("StructSpecifierWDEF", $1, $2, $4);}
				| STRUCT OptTag LC error RC {$$ = NULL;}
				| STRUCT Tag {$$ = newnode("StructSpecifier", $1, $2);}
				;
OptTag : ID {$$ = newnode("OptTag", $1);}
	   | /* empty */ {$$ = NULL;}
	   ;
Tag : ID {$$ = newnode("Tag", $1);}
	;
VarDec : ID {$$ = newnode("VarDec", $1);}
	   | ID VarDimList {$$ = newnode("VarDecARRAY", $1, $2);}
	   ;
VarDimList : LB INT RB {$$ = newnode("VarDimList", $2);}
		   | LB INT RB VarDimList {$$ = newnode("VarDimListMORE", $2, $4);}
           | LB error RB {raise_line_error(charno - 1, charno, _E_COLOR_ERR);}
           ;
FunDec : ID LP VarList RP {$$ = newnode("FunDecVL", $1, $3);}
	   | ID LP RP {$$ = newnode("FunDec", $1);}
	   | ID LP error RP {
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
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
