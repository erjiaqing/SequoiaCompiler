all: y.tab.c lexer.c extend.h node.c project1.h project2.h
	@echo "编译"
	@gcc y.tab.c -O2 -ly -ll -o ejq_cc -std=gnu99
	@echo "ejq_cc 是最终的可执行文件"
debug: y.tab.c lexer.c extend.h node.c
	@echo "编译(debug)"
	@gcc y.tab.c -ly -ll -o ejq_cc -D_DEBUG -std=gnu99
	@echo "ejq_cc 是最终的可执行文件"
y.tab.c: second.y
	@echo "文法分析器 second.y -> y.tab.c"
	@yacc -v second.y
lexer.c: first.l
	@echo "词法分析器 first.l -> lexer.c"
	@lex -o lexer.c first.l
cleanup:
	@echo "清理"
	@rm ejq_cc || echo "rm ejq_cc"
	@rm y.tab.c || echo "rm y.tab.cc"
	@rm lexer.c || echo "rm lexer.c"
	@rm y.output || echo "rm y.output"
