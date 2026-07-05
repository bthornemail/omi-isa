
all:
	gcc toolchain_main.c compiler.c parser.c ast.c lexer.c -o omi_toolchain

run:
	./omi_toolchain
