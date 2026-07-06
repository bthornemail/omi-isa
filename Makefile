CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g
COQC=coqc
COQFLAGS=-Q proof ''

VM_OBJ=main.o cpu.o boot.o loader.o compiler.o parser.o ast.o lexer.o asm.o omienv.o stream.o sector.o omi_dispatch.o omi_transport.o gauge_exec.o omi_mesh.o omicron.o omi_omion.o omi_receipt.o omi_sense.o omi_pg.o omi_orbit.o
TC_OBJ=toolchain_main.o loader.o compiler.o parser.o ast.o lexer.o omienv.o stream.o sector.o omi_dispatch.o omi_transport.o gauge_exec.o omi_mesh.o omicron.o omi_omion.o omi_receipt.o omi_sense.o omi_pg.o omi_orbit.o
LORA_OBJ=omi_transport_lora.o omi_transport_sim.o omi_probe.o omi_mesh.o omicron.o omi_omion.o omi_receipt.o omi_sense.o omi_pg.o omi_orbit.o

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
omi_mesh.o: omi_mesh.c omi_mesh.h omienv.h omi_transport.h
omicron.o: omicron.c omicron.h omienv.h omi_dispatch.h gauge_exec.h
omi_transport_lora.o: omi_transport_lora.c omi_transport_lora.h omi_transport.h
omi_omion.o: omi_omion.c omi_omion.h omienv.h omicron.h
omi_receipt.o: omi_receipt.c omi_receipt.h omienv.h omicron.h

omi_pg.o: omi_pg.c omi_pg.h omienv.h

omi_orbit.o: omi_orbit.c omi_orbit.h

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

WASM_SRCDIR=web
WASM_OBJDIR=web/build
WASM_OBJS=$(WASM_OBJDIR)/omienv.o $(WASM_OBJDIR)/stream.o $(WASM_OBJDIR)/sector.o \
           $(WASM_OBJDIR)/omi_dispatch.o $(WASM_OBJDIR)/omi_transport.o \
           $(WASM_OBJDIR)/gauge_exec.o $(WASM_OBJDIR)/cpu.o $(WASM_OBJDIR)/boot.o \
           $(WASM_OBJDIR)/omicron.o $(WASM_OBJDIR)/omi_omion.o $(WASM_OBJDIR)/omi_receipt.o

wasm: $(WASM_OBJDIR) $(WASM_OBJS) $(WASM_SRCDIR)/omi_web_bridge.c
	emcc -Os -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME=createModule \
		-s EXPORTED_FUNCTIONS='["_omi_web_init","_omi_web_push_byte","_omi_web_has_event","_omi_web_get_envelope","_omi_web_get_opcode","_omi_web_get_bitboard","_omi_web_pop_event","_omi_web_has_response","_omi_web_get_response","_omi_web_get_response_len","_omi_web_execute","_omi_web_inject_sensor","_omi_web_inject_event","_omi_web_inject_hardware","_malloc","_free"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","getValue","setValue"]' \
		-I. $(WASM_SRCDIR)/omi_web_bridge.c $(WASM_OBJS) \
		-o $(WASM_SRCDIR)/omi_wasm.js \
		--no-entry
	@echo "WASM build complete: web/omi_wasm.js + web/omi_wasm.wasm"

$(WASM_OBJDIR):
	mkdir -p $(WASM_OBJDIR)

$(WASM_OBJDIR)/omienv.o: omienv.c omienv.h
	emcc -Os -s WASM=1 -I. -c omienv.c -o $@

$(WASM_OBJDIR)/stream.o: stream.c stream.h omienv.h omi_dispatch.h cpu.h
	emcc -Os -s WASM=1 -I. -c stream.c -o $@

$(WASM_OBJDIR)/sector.o: sector.c sector.h omienv.h
	emcc -Os -s WASM=1 -I. -c sector.c -o $@

$(WASM_OBJDIR)/omi_dispatch.o: omi_dispatch.c omi_dispatch.h omienv.h cpu.h isa.h gauge_exec.h
	emcc -Os -s WASM=1 -I. -c omi_dispatch.c -o $@

$(WASM_OBJDIR)/omi_transport.o: omi_transport.c omi_transport.h omienv.h
	emcc -Os -s WASM=1 -I. -c omi_transport.c -o $@

$(WASM_OBJDIR)/gauge_exec.o: gauge_exec.c gauge_exec.h omienv.h omi_dispatch.h
	emcc -Os -s WASM=1 -I. -c gauge_exec.c -o $@

$(WASM_OBJDIR)/cpu.o: cpu.c cpu.h isa.h
	emcc -Os -s WASM=1 -I. -c cpu.c -o $@

$(WASM_OBJDIR)/boot.o: boot.c cpu.h
	emcc -Os -s WASM=1 -I. -c boot.c -o $@

$(WASM_OBJDIR)/omicron.o: omicron.c omicron.h omienv.h omi_dispatch.h gauge_exec.h
	emcc -Os -s WASM=1 -I. -c omicron.c -o $@

$(WASM_OBJDIR)/omi_omion.o: omi_omion.c omi_omion.h omienv.h omicron.h
	emcc -Os -s WASM=1 -I. -c omi_omion.c -o $@

$(WASM_OBJDIR)/omi_receipt.o: omi_receipt.c omi_receipt.h omienv.h omicron.h
	emcc -Os -s WASM=1 -I. -c omi_receipt.c -o $@

$(WASM_OBJDIR)/omi_pg.o: omi_pg.c omi_pg.h omienv.h
	emcc -Os -s WASM=1 -I. -c omi_pg.c -o $@

$(WASM_OBJDIR)/omi_orbit.o: omi_orbit.c omi_orbit.h
	emcc -Os -s WASM=1 -I. -c omi_orbit.c -o $@

test_mesh: test_mesh.c omi_mesh.c omi_transport_sim.c omi_transport.c omienv.c
	$(CC) $(CFLAGS) -o $@ test_mesh.c omi_mesh.c omi_transport_sim.c omi_transport.c omienv.c
	./test_mesh

test_omicron: test_omicron.c omicron.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	$(CC) $(CFLAGS) -o $@ test_omicron.c omicron.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	./test_omicron

test_omion: test_omion.c omi_omion.c omicron.c omienv.c omi_dispatch.c omi_transport.c gauge_exec.c stream.c sector.c cpu.c boot.c
	$(CC) $(CFLAGS) -o $@ test_omion.c omi_omion.c omicron.c omienv.c omi_dispatch.c omi_transport.c gauge_exec.c stream.c sector.c cpu.c boot.c
	./test_omion

test_receipt: test_receipt.c omi_receipt.c omicron.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	$(CC) $(CFLAGS) -o $@ test_receipt.c omi_receipt.c omicron.c gauge_exec.c omi_dispatch.c omi_transport.c omienv.c stream.c sector.c cpu.c boot.c
	./test_receipt

test_face_chain:
	node web/test-face-chain.js

test_omi_sense: test_omi_sense.c omi_sense.c omienv.c
	$(CC) $(CFLAGS) -o $@ test_omi_sense.c omi_sense.c omienv.c
	./test_omi_sense

test_pg: test_pg.c omi_pg.c omienv.c
	$(CC) $(CFLAGS) -o $@ test_pg.c omi_pg.c omienv.c
	./test_pg

test_orbit: test_orbit.c omi_orbit.c
	$(CC) $(CFLAGS) -o $@ test_orbit.c omi_orbit.c
	./test_orbit

test: test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_face_chain

PROOF_TARGETS= \
	proof/AtomicKernelVNext.vo \
	proof/omi_pi_proof.vo \
	proof/omi_pi_bridge.vo

proof: $(PROOF_TARGETS)

proof/AtomicKernelVNext.vo: proof/AtomicKernelVNext.v
	$(COQC) $(COQFLAGS) $<

proof/omi_pi_proof.vo: proof/omi_pi_proof.v
	$(COQC) $(COQFLAGS) $<

proof/omi_pi_bridge.vo: proof/omi_pi_bridge.v proof/AtomicKernelVNext.vo proof/omi_pi_proof.vo
	$(COQC) $(COQFLAGS) $<

clean:
	rm -f *.o omi_vm omi_toolchain test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_pg test_orbit test.bin omi.log bootstrap-compiler.bin bootstrap-compiler.omi
	rm -rf $(WASM_OBJDIR) $(WASM_SRCDIR)/omi_wasm.js $(WASM_SRCDIR)/omi_wasm.wasm

.PHONY: all run run-tc bootstrap clean wasm proof test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_face_chain
