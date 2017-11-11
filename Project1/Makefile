all:
	lex first.l
	yacc second.y -v
	gcc y.tab.c -O2 -ly -ll
