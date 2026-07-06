CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g
CPPFLAGS=-Ilib
COQC=coqc
COQFLAGS=-Q proof ''
COQPROOFFLAGS=-Q . ''
BUILD_DIR=build

VM_OBJ=$(BUILD_DIR)/main.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/boot.o $(BUILD_DIR)/loader.o \
       $(BUILD_DIR)/compiler.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/ast.o \
       $(BUILD_DIR)/lexer.o $(BUILD_DIR)/asm.o $(BUILD_DIR)/omienv.o \
       $(BUILD_DIR)/stream.o $(BUILD_DIR)/sector.o $(BUILD_DIR)/omi_dispatch.o \
       $(BUILD_DIR)/omi_transport.o $(BUILD_DIR)/gauge_exec.o \
       $(BUILD_DIR)/omi_mesh.o $(BUILD_DIR)/omicron.o $(BUILD_DIR)/omi_omion.o \
       $(BUILD_DIR)/omi_receipt.o $(BUILD_DIR)/omi_sense.o $(BUILD_DIR)/omi_pg.o \
       $(BUILD_DIR)/omi_orbit.o
TC_OBJ=$(BUILD_DIR)/toolchain_main.o $(BUILD_DIR)/loader.o $(BUILD_DIR)/compiler.o \
       $(BUILD_DIR)/parser.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/lexer.o \
       $(BUILD_DIR)/omienv.o $(BUILD_DIR)/stream.o $(BUILD_DIR)/sector.o \
       $(BUILD_DIR)/omi_dispatch.o $(BUILD_DIR)/omi_transport.o \
       $(BUILD_DIR)/gauge_exec.o $(BUILD_DIR)/omi_mesh.o $(BUILD_DIR)/omicron.o \
       $(BUILD_DIR)/omi_omion.o $(BUILD_DIR)/omi_receipt.o \
       $(BUILD_DIR)/omi_sense.o $(BUILD_DIR)/omi_pg.o $(BUILD_DIR)/omi_orbit.o

all: omi_vm omi_toolchain

omi_vm: $(VM_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(VM_OBJ)

omi_toolchain: $(TC_OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(TC_OBJ)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/toolchain_main.o: toolchain_main.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: lib/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test_env: test/test_env.c lib/omienv.c lib/stream.c lib/sector.c lib/omi_dispatch.c lib/omi_transport.c lib/gauge_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_env

test_dispatch: test/test_dispatch.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c lib/gauge_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_dispatch

test_gauge_exec: test/test_gauge_exec.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_gauge_exec

test_radio_vm: test/test_radio_vm.c lib/omi_transport_sim.c lib/omi_probe.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c lib/gauge_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
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

bootstrap-compiler.bin: scripts/gen_bootstrap.py
	python3 scripts/gen_bootstrap.py bootstrap-compiler.bin

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
		-Ilib $(WASM_SRCDIR)/omi_web_bridge.c $(WASM_OBJS) \
		-o $(WASM_SRCDIR)/omi_wasm.js \
		--no-entry
	@echo "WASM build complete: web/omi_wasm.js + web/omi_wasm.wasm"

$(WASM_OBJDIR):
	mkdir -p $(WASM_OBJDIR)

$(WASM_OBJDIR)/omienv.o: lib/omienv.c lib/omienv.h
	emcc -Os -s WASM=1 -Ilib -c lib/omienv.c -o $@

$(WASM_OBJDIR)/stream.o: lib/stream.c lib/stream.h lib/omienv.h lib/omi_dispatch.h lib/cpu.h
	emcc -Os -s WASM=1 -Ilib -c lib/stream.c -o $@

$(WASM_OBJDIR)/sector.o: lib/sector.c lib/sector.h lib/omienv.h
	emcc -Os -s WASM=1 -Ilib -c lib/sector.c -o $@

$(WASM_OBJDIR)/omi_dispatch.o: lib/omi_dispatch.c lib/omi_dispatch.h lib/omienv.h lib/cpu.h lib/isa.h lib/gauge_exec.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_dispatch.c -o $@

$(WASM_OBJDIR)/omi_transport.o: lib/omi_transport.c lib/omi_transport.h lib/omienv.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_transport.c -o $@

$(WASM_OBJDIR)/gauge_exec.o: lib/gauge_exec.c lib/gauge_exec.h lib/omienv.h lib/omi_dispatch.h
	emcc -Os -s WASM=1 -Ilib -c lib/gauge_exec.c -o $@

$(WASM_OBJDIR)/cpu.o: lib/cpu.c lib/cpu.h lib/isa.h
	emcc -Os -s WASM=1 -Ilib -c lib/cpu.c -o $@

$(WASM_OBJDIR)/boot.o: lib/boot.c lib/cpu.h
	emcc -Os -s WASM=1 -Ilib -c lib/boot.c -o $@

$(WASM_OBJDIR)/omicron.o: lib/omicron.c lib/omicron.h lib/omienv.h lib/omi_dispatch.h lib/gauge_exec.h
	emcc -Os -s WASM=1 -Ilib -c lib/omicron.c -o $@

$(WASM_OBJDIR)/omi_omion.o: lib/omi_omion.c lib/omi_omion.h lib/omienv.h lib/omicron.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_omion.c -o $@

$(WASM_OBJDIR)/omi_receipt.o: lib/omi_receipt.c lib/omi_receipt.h lib/omienv.h lib/omicron.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_receipt.c -o $@

$(WASM_OBJDIR)/omi_pg.o: lib/omi_pg.c lib/omi_pg.h lib/omienv.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_pg.c -o $@

$(WASM_OBJDIR)/omi_orbit.o: lib/omi_orbit.c lib/omi_orbit.h
	emcc -Os -s WASM=1 -Ilib -c lib/omi_orbit.c -o $@

test_mesh: test/test_mesh.c lib/omi_mesh.c lib/omi_transport_sim.c lib/omi_transport.c lib/omienv.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_mesh

test_omicron: test/test_omicron.c lib/omicron.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_omicron

test_omion: test/test_omion.c lib/omi_omion.c lib/omicron.c lib/omienv.c lib/omi_dispatch.c lib/omi_transport.c lib/gauge_exec.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_omion

test_receipt: test/test_receipt.c lib/omi_receipt.c lib/omicron.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_receipt

test_face_chain:
	node web/test-face-chain.js

test_omi_sense: test/test_omi_sense.c lib/omi_sense.c lib/omienv.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_omi_sense

test_pg: test/test_pg.c lib/omi_pg.c lib/omienv.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_pg

test_orbit: test/test_orbit.c lib/omi_orbit.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^
	./test_orbit

test: test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_face_chain

PROOF_TARGETS= \
	proof/AtomicKernel.vo \
	proof/AtomicKernelVNext.vo \
	proof/DiagonalClosure.vo \
	proof/FiniteIncidence.vo \
	proof/BQFBridge.vo \
	proof/MetricProjection.vo \
	proof/PiProjection.vo \
	proof/OMI_Exports.vo \
	proof/omi_pi_proof.vo \
	proof/omi_pi_bridge.vo \
	proof/delta_orbit_theory.vo \
	proof/functorial_semantics.vo \
	proof/coalgebraic_bisimulation.vo \
	proof/OMI_bialgebra.vo

proof: $(PROOF_TARGETS)

proof/AtomicKernel.vo: proof/AtomicKernel.v
	cd proof && $(COQC) $(COQPROOFFLAGS) AtomicKernel.v

proof/AtomicKernelVNext.vo: proof/AtomicKernelVNext.v proof/AtomicKernel.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) AtomicKernelVNext.v

proof/DiagonalClosure.vo: proof/DiagonalClosure.v
	cd proof && $(COQC) $(COQPROOFFLAGS) DiagonalClosure.v

proof/FiniteIncidence.vo: proof/FiniteIncidence.v proof/DiagonalClosure.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) FiniteIncidence.v

proof/BQFBridge.vo: proof/BQFBridge.v proof/DiagonalClosure.vo proof/FiniteIncidence.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) BQFBridge.v

proof/MetricProjection.vo: proof/MetricProjection.v proof/FiniteIncidence.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) MetricProjection.v

proof/PiProjection.vo: proof/PiProjection.v proof/DiagonalClosure.vo proof/BQFBridge.vo proof/MetricProjection.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) PiProjection.v

proof/OMI_Exports.vo: proof/OMI_Exports.v proof/DiagonalClosure.vo proof/FiniteIncidence.vo proof/BQFBridge.vo proof/MetricProjection.vo proof/PiProjection.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) OMI_Exports.v

proof/omi_pi_proof.vo: proof/omi_pi_proof.v proof/OMI_Exports.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) omi_pi_proof.v

proof/omi_pi_bridge.vo: proof/omi_pi_bridge.v proof/AtomicKernelVNext.vo proof/DiagonalClosure.vo proof/PiProjection.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) omi_pi_bridge.v

proof/delta_orbit_theory.vo: proof/delta_orbit_theory.v
	cd proof && $(COQC) $(COQPROOFFLAGS) delta_orbit_theory.v

proof/functorial_semantics.vo: proof/functorial_semantics.v proof/delta_orbit_theory.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) functorial_semantics.v

proof/coalgebraic_bisimulation.vo: proof/coalgebraic_bisimulation.v proof/delta_orbit_theory.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) coalgebraic_bisimulation.v

proof/OMI_bialgebra.vo: proof/OMI_bialgebra.v proof/delta_orbit_theory.vo proof/functorial_semantics.vo
	cd proof && $(COQC) $(COQPROOFFLAGS) OMI_bialgebra.v

clean:
	rm -f *.o omi_vm omi_toolchain test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test.bin omi.log bootstrap-compiler.bin bootstrap-compiler.omi
	rm -rf $(BUILD_DIR)
	rm -rf $(WASM_OBJDIR) $(WASM_SRCDIR)/omi_wasm.js $(WASM_SRCDIR)/omi_wasm.wasm

.PHONY: all run run-tc bootstrap clean wasm proof test test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_face_chain
