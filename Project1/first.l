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
"//"{ANYWORD}"\n" {
    yylval = NULL; lineno++;/* skip blank */
}
[\t ] {
    yylval = NULL;charno++; /* skip blank */
}
{TYPE} {
    char buf[1024];
    sprintf(buf, "TYPE: %s", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return TYPE;
}
{IF} {yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));charno += strlen(yytext);return IF;}
{ELSE} {yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));charno += strlen(yytext);return ELSE;}
{WHILE} {yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));charno += strlen(yytext);return WHILE;}
{RETURN} {yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));charno += strlen(yytext);return RETURN;}
{STRUCT} {yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));charno += strlen(yytext);return STRUCT;}
{INT} {
    char buf[1024];
    sprintf(buf, "INT: (DEC)%s", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return INT;
}
{HEXINT} {
    char buf[1024];
    int val;
    sscanf(yytext, "%x", &val);
    sprintf(buf, "INT: (HEX)%s => %d", yytext, val);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return INT;
}
{OCTINT} {
    char buf[1024];
    int val;
    sscanf(yytext, "%o", &val);
    sprintf(buf, "INT: (OCT)%s => %d", yytext, val);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return INT;
}
{ERRHEX} {
    char buf[1024];
    int val;
    sprintf(buf, "<<Error Type A.0>> Nya? ``%s'' might be a wrong hex integer.", yytext);
    yyerror(buf);
    sprintf(buf, "INT: (HEX)%s => ERR", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return INT;
}
{ERROCT} {
    char buf[1024];
    int val;
    sprintf(buf, "<<Error Type A.0>> Nya? ``%s'' might be a wrong oct integer.", yytext);
    yyerror(buf);
    sprintf(buf, "INT: (OCT)%s => ERR", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return INT;
}
{FLOAT} {
    char buf[1024];
    sprintf(buf, "FLOAT: %s => %.8lf", yytext, atof(yytext));
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return FLOAT;
}
{ID} {
    char buf[1024];
    sprintf(buf, "ID: %s", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return ID;
}
"/*" {yylval = NULL;charno += strlen(yytext);BEGIN(comment);}
<comment>"*"+"/" {yylval = NULL;charno += 2;BEGIN(INITIAL);}
<comment>\n {yylval = NULL;lineno++;charno = 1;}
<comment>. {yylval = NULL;charno += 1;/* skip */}
";" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return SEMI;
}
"," {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return COMMA;
}
"=" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return ASSIGNOP;
}
">"|"<"|">="|"<="|"=="|"!=" {
    char buf[1024];
    sprintf(buf, "RELOP: %s", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return RELOP;
}
"+" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return PLUS;
}
"-" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return MINUS;}
"*" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return STAR;}
"/" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return DIV;}
"&&" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return AND;}
"||" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return OR;}
"." {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return DOT;}
"!" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return NOT;}
"(" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return LP;}
")" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return RP;}
"[" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return LB;}
"]" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return RB;}
"{" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return LC;}
"}" {
    yylval = nnewnode(yytext, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return RC;}
\n {
    yylval = NULL;
    lineno++;
    charno = 1;
    /* skip blank */
}
{ERRWORD} {
    char buf[1024];
    sprintf(buf, "<<Error Type A.1>> Nya! I can recognize ``%s'', but why?", yytext);
    yyerror(buf);
    sprintf(buf, "ERRWORD: %s", yytext);
    yylval = nnewnode(buf, lineno, charno, lineno, charno + strlen(yytext));
    charno += strlen(yytext);
    return ID;
}
. {
    char buf[1024];
    charno += strlen(yytext);
    sprintf(buf, "<<Error Type A.2>> Nya! I cannot recognize ``%c'', wtf!", yytext[0]);
    yylval = NULL;
    yyerror(buf); /* skip */
}
