CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

VM_OBJ=main.o cpu.o boot.o loader.o compiler.o parser.o ast.o lexer.o asm.o omienv.o stream.o sector.o
TC_OBJ=toolchain_main.o loader.o compiler.o parser.o ast.o lexer.o omienv.o stream.o sector.o

all: omi_vm omi_toolchain

omi_vm: $(VM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(VM_OBJ)

omi_toolchain: $(TC_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TC_OBJ)

main.o: main.c cpu.h isa.h ast.h loader.h
cpu.o: cpu.c cpu.h isa.h
boot.o: boot.c cpu.h
loader.o: loader.c loader.h
compiler.o: compiler.c ast.h isa.h
parser.o: parser.c lexer.h ast.h
ast.o: ast.c ast.h
lexer.o: lexer.c lexer.h
asm.o: asm.c isa.h
toolchain_main.o: toolchain_main.c ast.h isa.h loader.h
omienv.o: omienv.c omienv.h
stream.o: stream.c stream.h omienv.h
sector.o: sector.c sector.h omienv.h

test_env: test_env.c omienv.c stream.c sector.c
	$(CC) $(CFLAGS) -o $@ test_env.c omienv.c stream.c sector.c
	./test_env

run: omi_vm
	./omi_vm programs/test.omi

run-tc: omi_toolchain
	./omi_toolchain programs/test.omi test.bin

bootstrap: omi_vm bootstrap-compiler.bin
	./omi_vm --boot bootstrap-compiler.bin bootstrap-compiler.omi /tmp/bt_self.bin
	cmp bootstrap-compiler.bin /tmp/bt_self.bin && echo "PASS: self-bootstrap" || echo "FAIL: self-bootstrap"
	./omi_vm --boot bootstrap-compiler.bin programs/test.omi /tmp/bt_test.bin
	python3 -c "assert open('/tmp/bt_test.bin','rb').read()==b'', 'test.omi should produce 0 words'; print('PASS: test.omi -> 0 words')"
	for f in programs/init.omi programs/kernel.omi programs/test.omi; do \
		./omi_vm $$f 2>/dev/null | grep -q halted && echo "PASS: $$f" || echo "FAIL: $$f"; \
	done

bootstrap-compiler.bin: gen_bootstrap.py
	python3 gen_bootstrap.py bootstrap-compiler.bin

clean:
	rm -f *.o omi_vm omi_toolchain test_env test.bin omi.log bootstrap-compiler.bin bootstrap-compiler.omi

.PHONY: all run run-tc bootstrap clean test_env
