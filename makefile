source = miniC
$(source).out: $(source).l $(source).y
	yacc -d -v $(source).y
	lex $(source).l
	g++ -o $(source).out lex.yy.c y.tab.c ast.c
clean:
	rm lex.yy.c y.tab.c y.tab.h $(source).out y.output
