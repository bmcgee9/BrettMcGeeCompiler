# BrettMcGeeCompiler
Brett McGee
cs57
Dartmouth ID: F0055ZJ

Compiler for CS57

# How to run: 

1. Call `make`

2.  Call `make p1Test` to run the assembly code generated from p1.c and p1.ll
    Call `make p2Test` to run the assembly code generated from p2.c and p2.ll
    Call `make p3Test` to run the assembly code generated from p3.c and p3.ll
    Call `make p4Test` to run the assembly code generated from p4.c and p4.ll

3. To clean up, you can use `make clean_p1Test`, `make clean_p2Test` and so on for each one or you can run `make clean_All`. Then use `make clean_All`.

If you would like to run your own test files, call `make` and then `./miniC.out [insert your c file here] [insert your llvm file here] > [assembly code name]`. Then run `clang -pedantic -g -Wno-strict-prototypes -m32 ./final_tests/main.c -o [insert desired output file name] [assembly code name]`, and now you can run `./[insert desired output file name]`.

Thank you for the great term!

