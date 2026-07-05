CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

VM_OBJ=main.o cpu.o boot.o loader.o compiler.o parser.o ast.o lexer.o asm.o omienv.o stream.o sector.o omi_dispatch.o omi_transport.o gauge_exec.o
TC_OBJ=toolchain_main.o loader.o compiler.o parser.o ast.o lexer.o omienv.o stream.o sector.o omi_dispatch.o omi_transport.o gauge_exec.o
LORA_OBJ=omi_transport_lora.o omi_transport_sim.o omi_probe.o

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
stream.o: stream.c stream.h omienv.h omi_dispatch.h cpu.h
sector.o: sector.c sector.h omienv.h
omi_dispatch.o: omi_dispatch.c omi_dispatch.h omienv.h cpu.h isa.h gauge_exec.h
omi_transport.o: omi_transport.c omi_transport.h omienv.h
gauge_exec.o: gauge_exec.c gauge_exec.h omienv.h omi_dispatch.h
omi_probe.o: omi_probe.c omi_probe.h omi_transport.h omi_dispatch.h cpu.h
omi_transport_sim.o: omi_transport_sim.c omi_transport_sim.h omi_transport.h
omi_transport_lora.o: omi_transport_lora.c omi_transport_lora.h omi_transport.h

test_env: test_env.c omienv.c stream.c sector.c omi_dispatch.c omi_transport.c gauge_exec.c
	$(CC) $(CFLAGS) -o $@ test_env.c omienv.c stream.c sector.c omi_dispatch.c omi_transport.c gauge_exec.c
	./test_env

test_dispatch: test_dispatch.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c gauge_exec.c
	$(CC) $(CFLAGS) -o $@ test_dispatch.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c gauge_exec.c
	./test_dispatch

test_gauge_exec: test_gauge_exec.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	$(CC) $(CFLAGS) -o $@ test_gauge_exec.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	./test_gauge_exec

test_radio_vm: test_radio_vm.c omi_transport_sim.c omi_probe.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c gauge_exec.c
	$(CC) $(CFLAGS) -o $@ test_radio_vm.c omi_transport_sim.c omi_probe.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c gauge_exec.c
	./test_radio_vm

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
	rm -f *.o omi_vm omi_toolchain test_env test_dispatch test_gauge_exec test_radio_vm test.bin omi.log bootstrap-compiler.bin bootstrap-compiler.omi

.PHONY: all run run-tc bootstrap clean test_env test_dispatch test_gauge_exec test_radio_vm
