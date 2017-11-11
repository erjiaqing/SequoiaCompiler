INT [0-9]|[1-9]([0-9]+)
FLOAT	(({INT}(\.{DIGIT}*)?|\.{DIGIT}+)([eE][+-]?{DIGIT}+)|({INT}\.{DIGIT}+))
ID [a-zA-Z_]([a-zA-Z0-9_]*)
TYPE (int|float)
STRUCT (struct)
RETURN (return)
IF (if)
ELSE (else)
WHILE (while)
%%
[ ] { /* skip blank */ }
{TYPE} {return TYPE;}
{IF} {return IF;}
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{RETURN} {return RETURN;}
{STRUCT} {return STRUCT;}
{INT} {return INT;}
{FLOAT} {return FLOAT;}
{ID} {return ID;}
";" {return SEMI;}
"," {return COMMA;}
"=" {return ASSIGNOP;}
">"|"<"|">="|"<="|"=="|"!=" {return RELOP;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return STAR;}
"/" {return DIV;}
"&&" {return AND;}
"||" {return OR;}
"." {return DOT;}
"!" {return NOT;}
"(" {return LP;}
")" {return RP;}
"[" {return LB;}
"]" {return RB;}
"{" {return LC;}
"}" {return RC;}
\n|. {return yytext[0];}
