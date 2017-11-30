%error-verbose
%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "extend.h"
#include "node.c"

#include "trie.h"
#include "symbol.h"

extern FILE* yyin;
FILE* line_reader;

char line_buffer[65536];
int lineno = 0, charno = 1;
int legal = 1;

int yylex();
// error type B
void yyerror(const char *msg)
{
	legal = 0;
	fprintf(stderr, "error type B on line %d: %s\n", lineno, msg);
}

// error type A
void llerror(const char *msg)
{
	legal = 0;
	fprintf(stderr, "error type A on line %d: %s\n", lineno, msg);
}

void move_to_next_line()
{
	lineno++;charno = 1;
	fgets(line_buffer, 65535, line_reader);
}

void _raise_line_error(int from, int to, int level, char *note)
{
#ifdef _DEBUG
// 输出一些好看一点的错误信息，如果想看效果的话，make debug即可
	for (int i = 0; line_buffer[i]; i++)
	{
		if (i + 1 == from) fprintf(stderr, "%s", write_color(level));
		if (i + 1 == to) fprintf(stderr, "\033[0m");
		fprintf(stderr, "%c", line_buffer[i]);
	}
	for (int i = 0; i + 1 < to; i++)
	{
		if (i + 1 == from) fprintf(stderr, "%s^", write_color(level));
		else if (i + 1 > from) fprintf(stderr, "~");
		else fprintf(stderr, line_buffer[i] == '\t' ? "\t" : " ");
	}
	fprintf(stderr, "\033[0m\n");
	if (note)
	{
		for (int i = 0; i + 1 <= from; i++)
			fprintf(stderr, line_buffer[i] == '\t' ? "\t" : " ");
		fprintf(stderr, "%s\n", note);
	}
#endif
}

#define raise_line_error_4(a, b, c, d) _raise_line_error(a, b, c, d)
#define raise_line_error(a, b, c) _raise_line_error(a, b, c, NULL)

#define YYSTYPE node_star

/* 下面这一段是用来统计可变参数的个数的，来源是stackoverflow */

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

// 重写新建节点的伪函数，方便自己调用

#define nnewnode(_000, a, b, c, d) _newnode(_000, a, b, c, d, 0)
#define newnode(_000, ...) _newnode(_000, 0, 0, 0, 0, PP_NARG(__VA_ARGS__), ##__VA_ARGS__)

// 遍历，内存漏就漏吧= =

void travel(node_star rt, int lvl)
{
	if (!legal) {
		exit(1);
	}
	printf("[%3d:%3d]->[%3d:%3d]", rt->start_lineno, rt->start_pos, rt->end_lineno, rt->end_pos - 1);
	for (int i = 0; i < lvl; i++) printf("  ");
	printf("%s\n", rt->label);
	for (int i = 0; i < rt->soncnt; i++)
		if (rt->son[i])
			travel(rt->son[i], lvl + 1);
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
ExtDef : Specifier ExtDecList SEMI {$$ = newnode("ExtDef", $1, $2, $3);}
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
				| STRUCT OptTag LC error RC {yyerror("<<Error Type B.1>> Meow. Valid DefList expected.");}
				| STRUCT Tag {$$ = newnode("StructSpecifier", $1, $2);}
				;
OptTag : ID {$$ = newnode("OptTag", $1);}
	   | /* empty */ {$$ = NULL;}
	   ;
Tag : ID {$$ = newnode("Tag", $1);}
	;
VarDec : ID {$$ = newnode("VarDec", $1);}
	   | ID VarDimList {$$ = newnode("VarDec", $1, $2);}
	   ;
VarDimList : LB INT RB {$$ = newnode("VarDimList", $1, $2, $3);}
		   | LB INT RB VarDimList {$$ = newnode("VarDimList", $1, $2, $3, $4);}
           | LB error RB {raise_line_error(charno - 1, charno, _E_COLOR_ERR);}
           ;
FunDec : ID LP VarList RP {$$ = newnode("FunDec", $1, $2, $3, $4);}
	   | ID LP RP {$$ = newnode("FunDec", $1, $2, $3);}
	   | ID LP error RP {
//			yyerror("<<Error Type B.1>> Meow! Valid varList expected.");
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
		}
	   ;
VarList : ParamDec COMMA VarList {$$ = newnode("VarList", $1, $2, $3);}
		| ParamDec {$$ = newnode("VarList", $1);}
		| ParamDec error {yyerror("<<Error Type B.0>> `,' expected");yyerrok;}
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
	 | RETURN Exp SEMI {$$ = newnode("Stmt", $1, $2, $3);}
	 | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = newnode("Stmt", $1, $2, $3, $4, $5);}
	 | IF LP Exp RP Stmt ELSE Stmt {$$ = newnode("Stmt", $1, $2, $3, $4, $5, $6, $7);}
	 | IF LP error RP {
//			yyerror("<<Error Type B.1>> Meow! Valid expression required.");
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
		}
	 | WHILE LP Exp RP Stmt {$$ = newnode("Stmt", $1, $2, $3, $4, $5);}
	 | Exp error {
//			yyerror("<<Error Type B.0>> Meow? ``;'' is expected");
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, ";");
		}
	 | IF error {
//			yyerror("<<Error Type B.0>> Meow? ``('' is required");
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, "(");
		}
	 | WHILE error {
//			yyerror("<<Error Type B.0>> Meow? ``('' is required");
			raise_line_error_4(charno, charno + 1, _E_COLOR_ERR, ")");
		}
	 | WHILE LP error RP{
//			yyerror("<<Error Type B.1>> Meow! Wtf is this? I can't read this expression.");
			raise_line_error(charno - 1, charno, _E_COLOR_ERR);
		}
	 ;
DefList : Def DefList {$$ = newnode("DefList", $1, $2);}
		| /* empty */ {$$ = NULL;}
		;
Def : Specifier DecList SEMI {$$ = newnode("Def", $1, $2, $3);}
	;
DecList : Dec {$$ = newnode("DecList", $1);}
		| Dec COMMA DecList {$$ = newnode("DecList", $1, $2, $3);}
		;
Dec : VarDec ASSIGNOP Exp {$$ = newnode("VarDec", $1, $2, $3);}
	| VarDec {$$ = newnode("Dec", $1);}
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
	| ID LP error RP{
//		yyerror("<<Error Type B.1>> Meow! Valid argument expression required.");
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| ID LP RP {$$ = newnode("Exp", $1, $2, $3);}
	| Exp LB Exp RB {$$ = newnode("Exp", $1, $2, $3, $4);}
	| Exp LB error RB {
//		yyerror("<<Error Type B.1>> Meow! Valid expression required.");
		raise_line_error(charno - 1, charno, _E_COLOR_ERR);
	}
	| Exp DOT ID {$$ = newnode("Exp", $1, $2, $3);}
	| ID {$$ = newnode("Exp", $1);}
	| INT {$$ = newnode("Exp", $1);}
	| FLOAT {$$ = newnode("Exp", $1);}
	;
Args : Exp COMMA Args {$$ = newnode("Args", $1, $2, $3);}
	 | Exp {$$ = newnode("Args", $1);}
	 ;
%%
#include "lexer.c"

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		yyin = fopen(argv[1], "r");
		line_reader = fopen(argv[1], "r");
	}
	else
	{
		printf("Fatal: %sNo input file.\033[0m\n", write_color(_E_COLOR_ERR));
		return -1;
	}
	move_to_next_line();
	yyparse();
}
