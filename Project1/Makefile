all: y.tab.c lexer.c extend.h node.c
	$(info Building 'ejq_cc'...)
	gcc y.tab.c -O2 -ly -ll -o ejq_cc
	$(info 'ejq_cc' is parser)
debug: y.tab.c lexer.c extend.h node.c
	$(info Building 'ejq_cc' with DEBUG...)
	gcc y.tab.c -ly -ll -o ejq_cc -D_DEBUG
y.tab.c: second.y
	$(info Building second.y -> y.tab.c...)
	yacc -v second.y
lexer.c: first.l
	$(info Building first.l -> lexer.c...)
	lex -o lexer.c first.l
cleanup:
	$(info Cleanning Up...)
	rm ejq_cc
	rm y.tab.c
	rm lexer.c
	rm y.output
