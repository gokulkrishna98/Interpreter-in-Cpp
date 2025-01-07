lexer:
	clang++ -c lexer.cpp -o lexer.out -std=c++23
repl:
	clang++ -c repl.cpp -o repl.out -std=c++23
interp:
	clang++ main.cpp lexer.out repl.out -o interp.out -std=c++23 
all:
	make lexer
	make repl
	make interp
run:
	./interp.out
clean:
	rm -f interp.out lexer.out repl.out