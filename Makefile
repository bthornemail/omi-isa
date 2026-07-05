CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

VM_OBJ=main.o cpu.o boot.o compiler.o parser.o ast.o lexer.o asm.o
TC_OBJ=toolchain_main.o compiler.o parser.o ast.o lexer.o

all: omi_vm omi_toolchain

omi_vm: $(VM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(VM_OBJ)

omi_toolchain: $(TC_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TC_OBJ)

main.o: main.c cpu.h isa.h ast.h
cpu.o: cpu.c cpu.h isa.h
boot.o: boot.c cpu.h
compiler.o: compiler.c ast.h isa.h
parser.o: parser.c lexer.h ast.h
ast.o: ast.c ast.h
lexer.o: lexer.c lexer.h
asm.o: asm.c isa.h
toolchain_main.o: toolchain_main.c ast.h isa.h

run: omi_vm
	./omi_vm programs/test.omi

run-tc: omi_toolchain
	./omi_toolchain programs/test.omi test.bin

clean:
	rm -f *.o omi_vm omi_toolchain test.bin omi.log

.PHONY: all run run-tc clean
