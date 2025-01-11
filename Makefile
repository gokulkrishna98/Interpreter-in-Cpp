lexer:
	clang++ -c lexer.cpp -o lexer.out -std=c++23
parser:
	clang++ -c parser.cpp -o parser.out -std=c++23 
repl:
	clang++ -c repl.cpp -o repl.out -std=c++23
interp:
	clang++ main.cpp lexer.out repl.out parser.out -o interp.out \
		-std=c++23 \
		-I/usr/include -L/usr/lib -lfmt
all:
	make lexer
	make parser
	make repl
	make interp
run:
	./interp.out
clean:
	rm -f interp.out lexer.out repl.out parser.out