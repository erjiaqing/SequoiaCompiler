all: y.tab.c lex.yy.c
	$(info Building 'ejq_cc'...)
	gcc y.tab.c -O2 -ly -ll -o ejq_cc
y.tab.c: second.y
	$(info Building second.y -> y.tab.c...)
	yacc -v second.y
lex.yy.c: first.l
	$(info Building first.l -> lex.yy.c...)
	lex first.l
