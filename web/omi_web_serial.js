class OMIWebSerialTransport {
    constructor() {
        this.port = null;
        this.reader = null;
        this.writer = null;
        this.connected = false;
        this.onEnvelope = null;
        this.onLog = null;
    }

    async connect(baudRate = 115200) {
        try {
            this.port = await navigator.serial.requestPort();
            await this.port.open({
                baudRate: baudRate,
                dataBits: 8,
                stopBits: 1,
                parity: 'none',
                flowControl: 'none'
            });
            this.connected = true;
            this.log('info', 'Serial connected');
            this.readLoop();
            return true;
        } catch (err) {
            this.log('error', 'Serial connect failed: ' + err.message);
            return false;
        }
    }

    async disconnect() {
        this.connected = false;
        if (this.reader) { try { await this.reader.cancel(); } catch(e) {} this.reader = null; }
        if (this.port) { try { await this.port.close(); } catch(e) {} this.port = null; }
        this.log('info', 'Disconnected');
    }

    async readLoop() {
        if (!this.port || !this.port.readable) return;
        try {
            this.reader = this.port.readable.getReader();
            while (true) {
                const { value, done } = await this.reader.read();
                if (done) break;
                for (const byte of value) {
                    if (typeof Module !== 'undefined' && Module._omi_web_push_byte) {
                        Module._omi_web_push_byte(byte);
                        if (Module._omi_web_has_event()) {
                            this.handleEnvelope();
                        }
                    }
                }
            }
        } catch (err) {
            if (this.connected) this.log('error', 'Serial read error: ' + err.message);
        } finally {
            if (this.reader) { this.reader.releaseLock(); this.reader = null; }
        }
    }

    handleEnvelope() {
        if (typeof Module === 'undefined') return;
        const ptr = Module._omi_web_get_envelope();
        if (!ptr) return;
        const buf = new Uint8Array(Module.HEAPU8.buffer, ptr, 64);
        const envelope = new Uint8Array(buf);
        const opcode = Module._omi_web_get_opcode();
        const bitboard = Module._omi_web_get_bitboard();
        const hasResp = Module._omi_web_has_response();
        let response = null;
        if (hasResp) {
            const rptr = Module._omi_web_get_response();
            const rlen = Module._omi_web_get_response_len();
            response = new Uint8Array(Module.HEAPU8.buffer, rptr, rlen);
            response = new Uint8Array(response);
        }
        if (this.onEnvelope) {
            this.onEnvelope({ envelope, opcode, bitboard, response });
        }
        Module._omi_web_pop_event();
    }

    async send(data) {
        if (!this.connected || !this.port || !this.port.writable) return false;
        try {
            const writer = this.port.writable.getWriter();
            await writer.write(data);
            writer.releaseLock();
            this.log('tx', `Sent ${data.length} bytes`);
            return true;
        } catch (err) {
            this.log('error', 'Send failed: ' + err.message);
            return false;
        }
    }

    async sendEnvelope(envelope) {
        const data = envelope.length === 64 ? envelope : new Uint8Array(64);
        if (envelope.length !== 64) data.set(envelope.slice(0, 64));
        return this.send(data);
    }

    log(level, msg) {
        if (this.onLog) this.onLog(level, msg);
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { OMIWebSerialTransport };
}
