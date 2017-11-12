%x comment
INT [0-9]|[1-9]([0-9]+)
HEXINT 0[xX][0-9a-fA-F]+
OCTINT 0[0-7]+
ERROCT 0[0-9]+
ERRHEX 0[xX][0-9a-zA-Z_]+
DIGIT [0-9]
FLOAT	(({INT}(\.{DIGIT}*)?|\.{DIGIT}+)([eE][+-]?{DIGIT}+)|({INT}?\.{DIGIT}+)|({INT}\.{DIGIT}*))
ID [a-zA-Z_]([a-zA-Z0-9_]*)
TYPE (int|float)
STRUCT (struct)
RETURN (return)
IF (if)
ELSE (else)
WHILE (while)
ERRWORD ([a-zA-Z0-9_]+)
ANYWORD (.*)
ANYWORD_INCLUDE_NEWLINE ((.|\n)*)
%%
"//"{ANYWORD}"\n" {yylval = NULL; lineno++;/* skip blank */}
[\t ] {yylval = NULL; /* skip blank */}
{TYPE} {char buf[1024];sprintf(buf, "TYPE: %s", yytext);yylval = nnewnode(buf);return TYPE;}
{IF} {yylval = nnewnode(yytext);return IF;}
{ELSE} {yylval = nnewnode(yytext);return ELSE;}
{WHILE} {yylval = nnewnode(yytext);return WHILE;}
{RETURN} {yylval = nnewnode(yytext);return RETURN;}
{STRUCT} {yylval = nnewnode(yytext);return STRUCT;}
{INT} {char buf[1024];sprintf(buf, "INT: (DEC)%s", yytext);yylval = nnewnode(buf);return INT;}
{HEXINT} {char buf[1024];int val;sscanf(yytext, "%x", &val);sprintf(buf, "INT: (HEX)%s => %d", yytext, val);yylval = nnewnode(buf);return INT;}
{OCTINT} {char buf[1024];int val;sscanf(yytext, "%o", &val);sprintf(buf, "INT: (OCT)%s => %d", yytext, val);yylval = nnewnode(buf);return INT;}
{ERRHEX} {char buf[1024];int val;sprintf(buf, "Nya? ``%s'' might be a wrong hex integer.", yytext);yyerror(buf);sprintf(buf, "INT: (HEX)%s => ERR", yytext);yylval = nnewnode(buf);return INT;}
{ERROCT} {char buf[1024];int val;sprintf(buf, "Nya? ``%s'' might be a wrong oct integer.", yytext);yyerror(buf);sprintf(buf, "INT: (OCT)%s => ERR", yytext);yylval = nnewnode(buf);return INT;}
{FLOAT} {char buf[1024];sprintf(buf, "FLOAT: %s => %.8lf", yytext, atof(yytext));yylval = nnewnode(buf);return FLOAT;}
{ID} {char buf[1024];sprintf(buf, "ID: %s", yytext);yylval = nnewnode(buf);return ID;}
"/*" {yylval = NULL;BEGIN(comment);}
<comment>"*"+"/" {yylval = NULL;BEGIN(INITIAL);}
<comment>\n {yylval = NULL;lineno++;}
<comment>. {yylval = NULL;/* skip */}
";" {yylval = nnewnode(yytext);return SEMI;}
"," {yylval = nnewnode(yytext);return COMMA;}
"=" {yylval = nnewnode(yytext);return ASSIGNOP;}
">"|"<"|">="|"<="|"=="|"!=" {char buf[1024];sprintf(buf, "RELOP: %s", yytext);yylval = nnewnode(buf);return RELOP;}
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
{ERRWORD} {char buf[1024];sprintf(buf, "Nya! I can recognize ``%s'', but why?", yytext);yyerror(buf);sprintf(buf, "ERRWORD: %s", yytext);yylval = nnewnode(buf);return ID;}
. {char buf[1024];sprintf(buf, "Nya! I cannot recognize ``%c'', wtf!", yytext[0]);yylval = NULL; yyerror(buf); /* skip */}
