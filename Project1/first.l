INT [0-9]|[1-9]([0-9]+)
HEXINT 0[xX][0-9a-fA-F]+
OCTINT 0[0-7]+
DIGIT [0-9]
FLOAT	(({INT}(\.{DIGIT}*)?|\.{DIGIT}+)([eE][+-]?{DIGIT}+)|({INT}?\.{DIGIT}+)|({INT}\.{DIGIT}*))
ID [a-zA-Z_]([a-zA-Z0-9_]*)
TYPE (int|float)
STRUCT (struct)
RETURN (return)
IF (if)
ELSE (else)
WHILE (while)
%%
[ ] {yylval = NULL; /* skip blank */ }
{TYPE} {char buf[1024];sprintf(buf, "TYPE: %s", yytext);yylval = nnewnode(buf);return TYPE;}
{IF} {yylval = nnewnode(yytext);return IF;}
{ELSE} {yylval = nnewnode(yytext);return ELSE;}
{WHILE} {yylval = nnewnode(yytext);return WHILE;}
{RETURN} {yylval = nnewnode(yytext);return RETURN;}
{STRUCT} {yylval = nnewnode(yytext);return STRUCT;}
{INT} {char buf[1024];sprintf(buf, "INT: (DEC)%s", yytext);yylval = nnewnode(buf);return INT;}
{HEXINT} {char buf[1024];int val;sscanf(yytext, "%x", &val);sprintf(buf, "INT: (HEX)%s => %d", yytext, val);yylval = nnewnode(buf);}
{OCTINT} {char buf[1024];int val;sscanf(yytext, "%o", &val);sprintf(buf, "INT: (OCT)%s => %d", yytext, val);yylval = nnewnode(buf);}
{FLOAT} {char buf[1024];sprintf(buf, "FLOAT: %s => %.8lf", yytext, atof(yytext));yylval = nnewnode(buf);return FLOAT;}
{ID} {char buf[1024];sprintf(buf, "ID: %s", yytext);yylval = nnewnode(buf);return ID;}
";" {yylval = nnewnode(yytext);return SEMI;}
"," {yylval = nnewnode(yytext);return COMMA;}
"=" {yylval = nnewnode(yytext);return ASSIGNOP;}
">"|"<"|">="|"<="|"=="|"!=" {yylval = nnewnode(yytext);return RELOP;}
"+" {yylval = nnewnode(yytext);return PLUS;}
"-" {yylval = nnewnode(yytext);return MINUS;}
"*" {yylval = nnewnode(yytext);return STAR;}
"/" {yylval = nnewnode(yytext);return DIV;}
"&&" {yylval = nnewnode(yytext);return AND;}
"||" {yylval = nnewnode(yytext);return OR;}
"." {yylval = nnewnode(yytext);return DOT;}
"!" {yylval = nnewnode(yytext);return NOT;}
"(" {yylval = nnewnode(yytext);return LP;}
")" {yylval = nnewnode(yytext);return RP;}
"[" {yylval = nnewnode(yytext);return LB;}
"]" {yylval = nnewnode(yytext);return RB;}
"{" {yylval = nnewnode(yytext);return LC;}
"}" {yylval = nnewnode(yytext);return RC;}
\n {yylval = NULL; lineno++;/* skip blank */ }
. {yylval = NULL; /* skip */}
