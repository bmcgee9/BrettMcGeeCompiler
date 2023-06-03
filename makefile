source = miniC
TEST = ./final_tests/p1
FUNCS = runner.cpp y.tab.c lex.yy.c ast.cpp semanticC.cpp optimizer.cpp codegen.cpp
CFLAGS = -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/
$(source).out: miniC.l miniC.y
	yacc -d -v miniC.y
	lex miniC.l
	clang++ $(CFLAGS) $(FUNCS) -o $@
# $(source).out: $(source).l $(source).y

p1Test:
	./miniC.out ./final_tests/p1.c ./final_tests/p1.ll > p1final.s
	clang -pedantic -g -Wno-strict-prototypes -m32 ./final_tests/main.c -o p1mainfinal.o p1final.s
	./p1mainfinal.o

clean_p1Test:
	rm p1final.s p1mainfinal.o

p2Test:
	./miniC.out ./final_tests/p2.c ./final_tests/p2.ll > p2final.s 2>&1
	clang -pedantic -g -Wno-strict-prototypes -m32 ./final_tests/main.c -o p2mainfinal.o p2final.s
	./p2mainfinal.o

clean_p2Test:
	rm p2final.s p2mainfinal.o

p3Test:
	./miniC.out ./final_tests/p3.c ./final_tests/p3.ll > p3final.s 2>&1
	clang -pedantic -g -Wno-strict-prototypes -m32 ./final_tests/main.c -o p3mainfinal.o p3final.s
	./p3mainfinal.o

clean_p3Test:
	rm p3final.s p3mainfinal.o

p4Test:
	./miniC.out ./final_tests/p4.c ./final_tests/p4.ll > p4final.s 2>&1
	clang -pedantic -g -Wno-strict-prototypes -m32 ./final_tests/main.c -o p4mainfinal.o p4final.s
	./p4mainfinal.o

clean_p4Test:
	rm p4final.s p4mainfinal.o

clean:
	rm lex.yy.c y.tab.c y.tab.h $(source).out y.output

clean_All:
	make clean_p1Test
	make clean_p2Test
	make clean_p3Test
	make clean_p4Test

llvm_file: $(TEST).c
	clang -S -emit-llvm $(TEST).c -o test_branches.ll

reg_alloc: 
	clang++ -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/ -c codegen.cpp
	clang++ -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/ codegen.o -o reg_alloc.out

clean_reg_alloc:
	rm codegen.o reg_alloc.out


baseP1Test:
	clang -pedantic -g -m32 ./final_tests/main.c -o main.o ./final_tests/p1.c

mainTest:
	clang -m32 main.c -o main.o ass.s

clean_pTests:
	rm p*main.o

baseTest:
	clang -m32 main.c -o main.o test.s
branchTest:
	clang -pedantic -g -m32 ./test_branches/main.c -o main.o branches13.s
baseBranchTest:
	clang -pedantic -g -m32 ./test_branches/main.c -o main.o ./test_branches/test_branches.s

funcCallTest:
	clang -pedantic -g -m32 ./func_calls/main.c -o main.o funcCall.s
baseFuncCallTest:
	clang -pedantic -g -m32 ./func_calls/main.c -o main.o ./func_calls/func_call.s
clean_maino:
	rm main.o

optTest:
	clang -pedantic -g `llvm-config-15 --cflags` -I /usr/include/llvm-c-15/ -c optimizer.c
	clang++ -pedantic -g `llvm-config-15 --cxxflags --ldflags --libs core` -I /usr/include/llvm-c-15/ optimizer.o -o $@

clean_optTest:
	rm optimizer.o optTest


