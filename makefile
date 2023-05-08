source = miniC
TEST = ./optest/test1
FUNCS = runner.c y.tab.c lex.yy.c ast.c semanticC.c optimizer.c
CFLAGS = -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/
$(source).out: $(source).l $(source).y
	yacc -d -v $(source).y
	lex $(source).l
	clang++ $(CFLAGS) $(FUNCS) -o $@
# $(source).out: $(source).l $(source).y
	
llvm_file: $(TEST).c
	clang -S -emit-llvm $(TEST).c -o test1.ll

optTest:
	clang -pedantic -g `llvm-config-15 --cflags` -I /usr/include/llvm-c-15/ -c optimizer.c
	clang++ -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/ optimizer.o -o $@

clean_optTest:
	rm optimizer.o optTest
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