source = miniC
$(source).out: $(source).l $(source).y
	yacc -d -v $(source).y
	lex $(source).l
	g++ -o $(source).out lex.yy.c y.tab.c ast.c -pedantic -g
clean:
	rm lex.yy.c y.tab.c y.tab.h $(source).out y.output

clean_test: 
	rm stest.out test.out
test:
	./$(source).out ./examples/p1.c > test.out 2>&1 
	./$(source).out ./examples/p2.c >> test.out 2>&1 
	./$(source).out ./examples/p3.c >> test.out 2>&1 
	./$(source).out ./examples/p4.c >> test.out 2>&1 
	./$(source).out ./examples/p5.c >> test.out 2>&1 

stest:
	./$(source).out ./semantic_test/p1.c > stest.out 2>&1 
	./$(source).out ./semantic_test/p2.c >> stest.out 2>&1 
	./$(source).out ./semantic_test/p3.c >> stest.out 2>&1 
	./$(source).out ./semantic_test/p4.c >> stest.out 2>&1 