%{
#include <stdio.h>
#include <string.h>
#include <math.h>
%}

DIGIT	[0-9]
NONZERO	[1-9]
ID_CHAR	[a-zA-Z_]
INT		0|([1-9][0-9]*)
FLOAT	(({INT}(\.{DIGIT}*)?|\.{DIGIT}+)([eE][+-]?{DIGIT}+)|({INT}\.{DIGIT}+))

%%

{FLOAT}	{
	/* float type*/
	printf("FLOAT \"%s\"\n", yytext);
}

{INT}	{
	/* int */
	printf("INT \"%s\" = %d\n", yytext, atoi(yytext));
}

{ID_CHAR}({ID_CHAR}|{DIGIT})*	{
	/* id */
	printf("ID \"%s\"\n", yytext);
}

";"	{
	printf("SEMI\n");
}

"," {
	printf("COMMA\n");
}

"+""=" {
	printf("ASSIGNOP\n");
}

"<="|">="|"<"|">"|"=="|"!="	{
	printf("RELOP %s\n", yytext);
}

%%

int main()
{
  printf("Give me your input:\n");
  yylex();
}
