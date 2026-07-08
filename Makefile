CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g
CPPFLAGS=-Ilib
BUILD_DIR=build
BIN_DIR=bin

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

all: $(BIN_DIR)/omi_vm $(BIN_DIR)/omi_toolchain

# Convenience aliases (build from root without path prefix)
omi_vm: $(BIN_DIR)/omi_vm
omi_toolchain: $(BIN_DIR)/omi_toolchain

$(BIN_DIR)/omi_vm: $(VM_OBJ) | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(VM_OBJ)

$(BIN_DIR)/omi_toolchain: $(TC_OBJ) | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(TC_OBJ)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/toolchain_main.o: toolchain_main.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: lib/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/test_env: test/test_env.c lib/omienv.c lib/stream.c lib/sector.c lib/omi_dispatch.c lib/omi_transport.c lib/gauge_exec.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_env: $(BIN_DIR)/test_env
	$(BIN_DIR)/test_env

$(BIN_DIR)/test_dispatch: test/test_dispatch.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c lib/gauge_exec.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_dispatch: $(BIN_DIR)/test_dispatch
	$(BIN_DIR)/test_dispatch

$(BIN_DIR)/test_gauge_exec: test/test_gauge_exec.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_gauge_exec: $(BIN_DIR)/test_gauge_exec
	$(BIN_DIR)/test_gauge_exec

$(BIN_DIR)/test_radio_vm: test/test_radio_vm.c lib/omi_transport_sim.c lib/omi_probe.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c lib/gauge_exec.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_radio_vm: $(BIN_DIR)/test_radio_vm
	$(BIN_DIR)/test_radio_vm

$(BIN_DIR)/test_mesh: test/test_mesh.c lib/omi_mesh.c lib/omi_transport_sim.c lib/omi_transport.c lib/omienv.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_mesh: $(BIN_DIR)/test_mesh
	$(BIN_DIR)/test_mesh

$(BIN_DIR)/test_omicron: test/test_omicron.c lib/omicron.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_omicron: $(BIN_DIR)/test_omicron
	$(BIN_DIR)/test_omicron

$(BIN_DIR)/test_omion: test/test_omion.c lib/omi_omion.c lib/omicron.c lib/omienv.c lib/omi_dispatch.c lib/omi_transport.c lib/gauge_exec.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_omion: $(BIN_DIR)/test_omion
	$(BIN_DIR)/test_omion

$(BIN_DIR)/test_receipt: test/test_receipt.c lib/omi_receipt.c lib/omicron.c lib/gauge_exec.c lib/omi_dispatch.c lib/omi_transport.c lib/omienv.c lib/stream.c lib/sector.c lib/cpu.c lib/boot.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_receipt: $(BIN_DIR)/test_receipt
	$(BIN_DIR)/test_receipt

$(BIN_DIR)/test_omi_sense: test/test_omi_sense.c lib/omi_sense.c lib/omienv.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_omi_sense: $(BIN_DIR)/test_omi_sense
	$(BIN_DIR)/test_omi_sense

$(BIN_DIR)/test_pg: test/test_pg.c lib/omi_pg.c lib/omienv.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_pg: $(BIN_DIR)/test_pg
	$(BIN_DIR)/test_pg

$(BIN_DIR)/test_orbit: test/test_orbit.c lib/omi_orbit.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

$(BIN_DIR)/test_lisp_handoff: test/test_lisp_handoff.c lib/loader.c lib/lexer.c lib/parser.c lib/ast.c lib/compiler.c lib/cpu.c lib/boot.c | $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

test_orbit: $(BIN_DIR)/test_orbit
	$(BIN_DIR)/test_orbit

test_lisp_handoff: $(BIN_DIR)/test_lisp_handoff
	$(BIN_DIR)/test_lisp_handoff

test_face_chain:
	node web/test-face-chain.js

test: test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_lisp_handoff test_face_chain

run: $(BIN_DIR)/omi_vm
	$(BIN_DIR)/omi_vm programs/test.omi

run-tc: $(BIN_DIR)/omi_toolchain
	$(BIN_DIR)/omi_toolchain programs/test.omi test.bin

bootstrap: $(BIN_DIR)/omi_vm bootstrap-compiler.bin
	$(BIN_DIR)/omi_vm --boot bootstrap-compiler.bin bootstrap-compiler.omi /tmp/bt_self.bin
	cmp bootstrap-compiler.bin /tmp/bt_self.bin && echo "PASS: self-bootstrap" || echo "FAIL: self-bootstrap"
	$(BIN_DIR)/omi_vm --boot bootstrap-compiler.bin programs/test.omi /tmp/bt_test.bin
	python3 -c "assert open('/tmp/bt_test.bin','rb').read()==b'', 'test.omi should produce 0 words'; print('PASS: test.omi -> 0 words')"
	for f in programs/init.omi programs/kernel.omi programs/test.omi; do \
		$(BIN_DIR)/omi_vm $$f 2>/dev/null | grep -q halted && echo "PASS: $$f" || echo "FAIL: $$f"; \
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

proof:
	@if [ -d ../omi-axioms ]; then \
		$(MAKE) -C ../omi-axioms proof; \
	else \
		echo "OMI proofs live in the omi-axioms repository."; \
	fi

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	touch $(BUILD_DIR)/.gitkeep $(BIN_DIR)/.gitkeep
	rm -f test.bin omi.log bootstrap-compiler.bin bootstrap-compiler.omi
	rm -rf $(WASM_OBJDIR) $(WASM_SRCDIR)/omi_wasm.js $(WASM_SRCDIR)/omi_wasm.wasm

.PHONY: all run run-tc bootstrap clean wasm proof test test_env test_dispatch test_gauge_exec test_radio_vm test_mesh test_omicron test_omion test_receipt test_omi_sense test_pg test_orbit test_lisp_handoff test_face_chain
