all: y.tab.c lexer.c extend.h node.c project1.h project2.h
	$(info Building 'ejq_cc'...)
	gcc y.tab.c -O2 -ly -ll -o ejq_cc -std=gnu99
	$(info 'ejq_cc' is parser)
debug: y.tab.c lexer.c extend.h node.c
	$(info Building 'ejq_cc' with DEBUG...)
	gcc y.tab.c -ly -ll -o ejq_cc -g -D_DEBUG -std=gnu99
y.tab.c: second.y
	$(info Building second.y -> y.tab.c...)
	yacc -v second.y
lexer.c: first.l
	$(info Building first.l -> lexer.c...)
	lex -o lexer.c first.l
cleanup:
	$(info Cleanning Up...)
	rm ejq_cc || echo "rm ejq_cc"
	rm y.tab.c || echo "rm y.tab.cc"
	rm lexer.c || echo "rm lexer.c"
	rm y.output || echo "rm y.output"
