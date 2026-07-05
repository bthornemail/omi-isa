const state = {
    wasmReady: false,
    serial: null,
    audio: null,
    recordedFrames: [],
    rxAudioQueue: [],
    audioCtx: null,
};

const GAUGE_ACTION_NAMES = {
    0: 'NONE',
    1: 'PLACE',
    2: 'REGISTER_INJECT',
    3: 'KERNEL_READ',
    4: 'RECORD_CLOSE',
    5: 'SYSTEM_WITNESS',
    6: 'SEAL',
    7: 'BOOT_PAGE',
    8: 'EXTERNAL_BRIDGE'
};

const GAUGE_ACTION_CLASSES = {
    1: 'place',
    2: 'inject',
    3: 'query',
    4: 'close'
};

function projectEnvelope(env, bitboard) {
    const gauge = env[0];
    const actionCode = bitboard ? ((bitboard >> 22) & 0xFF) : 0;
    const actionName = GAUGE_ACTION_NAMES[actionCode] || 'UNKNOWN';
    const cls = GAUGE_ACTION_CLASSES[actionCode] || '';

    const addr = Array.from(env.slice(0, 16))
        .map(b => b.toString(16).padStart(2, '0')).join(' ');
    const container = document.getElementById('projection-container');
    const entry = document.createElement('div');
    entry.className = 'proj-entry' + (cls ? ' ' + cls : '');
    entry.innerHTML = `
        <span class="proj-addr">${addr}</span>
        <span class="proj-action">${actionName}</span>
        <span class="proj-receipt">${bitboard ? '0x' + bitboard.toString(16).padStart(16, '0') : ''}</span>
    `;
    container.appendChild(entry);
    if (container.children.length > 100) container.firstChild.remove();
    container.scrollTop = container.scrollHeight;
}

// Mesh debug state
let meshDebugInterval = null;

function startMeshDebug() {
    if (meshDebugInterval) return;
    meshDebugInterval = setInterval(requestMeshStatus, 5000);
}

function stopMeshDebug() {
    if (meshDebugInterval) {
        clearInterval(meshDebugInterval);
        meshDebugInterval = null;
    }
}

function requestMeshStatus() {
    if (!state.serial || !state.serial.connected) {
        document.getElementById('mesh-routes').textContent = 'Disconnected';
        document.getElementById('mesh-queue').textContent = '\u2014';
        document.getElementById('mesh-dead').textContent = '\u2014';
        return;
    }
    const env = new Uint8Array(64);
    const preheader = [0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF];
    env.set(preheader, 0);
    env[8] = 0x03;
    state.serial.sendEnvelope(env);
}

function updateMeshDisplay(env) {
    if (env[8] !== 0x03) return;
    const routeCount = env[9];
    const queueDepth = env[10];
    const deadCount = env[11];
    document.getElementById('mesh-routes').textContent = routeCount.toString();
    document.getElementById('mesh-queue').textContent = queueDepth.toString();
    document.getElementById('mesh-dead').textContent = deadCount.toString();
    const routesEl = document.getElementById('mesh-route-list');
    routesEl.innerHTML = '';
    for (let i = 0; i < routeCount && i < 8; i++) {
        const off = 32 + i * 4;
        const dest = env[off];
        const nextHop = env[off + 1];
        const metric = env[off + 2];
        const seq = env[off + 3];
        const row = document.createElement('div');
        row.className = 'route-entry';
        row.textContent = `\u2192 ${dest} via ${nextHop} (metric ${metric}, seq ${seq})`;
        routesEl.appendChild(row);
    }
}

let viewerBridge = null;
let viewerVisible = false;

function toggleViewer() {
    const panel = document.getElementById('viewer-panel');
    const btn = document.getElementById('btn-toggle-viewer');
    if (!panel || !btn) return;
    viewerVisible = !viewerVisible;
    panel.classList.toggle('viewer-panel-hidden', !viewerVisible);
    panel.classList.toggle('viewer-panel-visible', viewerVisible);
    btn.textContent = viewerVisible ? '\u25BC 3D Viewer' : '\u25B6 3D Viewer';

    if (viewerVisible && viewerBridge && !viewerBridge.lazyInitDone) {
        const container = document.getElementById('twin-container');
        if (container) viewerBridge.initViewer(container);
    }

    if (viewerVisible && viewerBridge && viewerBridge.twinViewer) {
        viewerBridge.twinViewer.onResize();
    }
}

function updateViewerStats(frameData) {
    const t = frameData && frameData.twin;
    if (!t) return;
    document.getElementById('vwr-cycles').textContent = 'cycle: ' + (t.cycle || 0);
    document.getElementById('vwr-receipts').textContent = 'receipts: ' + ((t.summary && t.summary.filled) || 0);
    document.getElementById('vwr-filled').textContent = 'filled: ' + ((t.summary && t.summary.filled) || 0) + '/5040';
}

function log(level, msg) {
    const el = document.getElementById('sys-log');
    const ts = new Date().toISOString().slice(11, 23);
    const line = document.createElement('div');
    line.textContent = `[${ts}] [${level}] ${msg}`;
    el.appendChild(line);
    el.scrollTop = el.scrollHeight;
    el.querySelector('.placeholder')?.remove();
}

function rxLog(msg, cls = 'env-entry') {
    const el = document.getElementById('rx-log');
    const line = document.createElement('div');
    line.className = cls;
    line.textContent = msg;
    el.appendChild(line);
    el.scrollTop = el.scrollHeight;
    el.querySelector('.placeholder')?.remove();
}

function setStatus(id, online) {
    document.getElementById(id).className = 'status ' + (online ? 'online' : 'offline');
}

function hex(buf) {
    return Array.from(buf).map(b => b.toString(16).padStart(2, '0')).join(' ');
}

function updateInjectButtons(enabled) {
    document.getElementById('btn-inject-sensor').disabled = !enabled || !state.wasmReady;
    document.getElementById('btn-inject-event').disabled = !enabled || !state.wasmReady;
    document.getElementById('btn-inject-hardware').disabled = !enabled || !state.wasmReady;
    document.getElementById('btn-record').disabled = !enabled || !state.wasmReady;
}

function isAudioEnvelope(env) {
    return env[8] === 0x0A && env[9] === 0x01;
}

function unpackAudioSamples(env) {
    const samples = [];
    for (let i = 0; i < 16; i++) {
        const hi = env[32 + i * 2];
        const lo = env[32 + i * 2 + 1];
        let s = (hi << 8) | lo;
        if (s >= 0x8000) s -= 0x10000;
        samples.push(s);
    }
    return samples;
}

function playAudioQueue() {
    if (!state.audioCtx || state.rxAudioQueue.length === 0) return;
    const total = state.rxAudioQueue.reduce((a, f) => a + f.length, 0);
    if (total === 0) return;
    const f32 = new Float32Array(total);
    let idx = 0;
    for (const frame of state.rxAudioQueue) {
        for (const s of frame) f32[idx++] = s / 32768;
    }
    state.rxAudioQueue = [];
    const buf = state.audioCtx.createBuffer(1, f32.length, state.audioCtx.sampleRate);
    buf.copyToChannel(f32, 0);
    const src = state.audioCtx.createBufferSource();
    src.buffer = buf;
    src.connect(state.audioCtx.destination);
    src.start();
}

async function init() {
    const btnConnect = document.getElementById('btn-connect');
    const btnDisconnect = document.getElementById('btn-disconnect');
    const btnSensor = document.getElementById('btn-inject-sensor');
    const btnEvent = document.getElementById('btn-inject-event');
    const btnHardware = document.getElementById('btn-inject-hardware');
    const btnRecord = document.getElementById('btn-record');
    const btnStop = document.getElementById('btn-stop');

    if (typeof createModule !== 'function') {
        log('error', 'WASM module loader not found (omi_wasm.js)');
        btnConnect.disabled = true;
        return;
    }

    try {
        const Module = await createModule();
        Module._omi_web_init();
        state.wasmReady = true;
        setStatus('status-wasm', true);
        log('info', 'WASM module initialized');
        btnConnect.disabled = false;
        btnSensor.disabled = false;
        btnEvent.disabled = false;
        btnHardware.disabled = false;
        btnRecord.disabled = false;
    } catch (err) {
        log('error', 'WASM init failed: ' + err.message);
        btnConnect.disabled = true;
        return;
    }

    state.audioCtx = new (window.AudioContext || window.webkitAudioContext)();

    viewerBridge = new ViewerBridge();
    const facePanel = new FacePanel('face-panel');
    viewerBridge.setFacePanel(facePanel);

    state.serial = new OMIWebSerialTransport();
    state.serial.onLog = (level, msg) => log(level, msg);
    state.serial.onEnvelope = (evt) => {
        const hexStr = hex(evt.envelope);
        rxLog(`ENVELOPE opcode=0x${evt.opcode.toString(16).padStart(2,'0')} bitboard=0x${evt.bitboard.toString(16).padStart(16,'0')}`);
        rxLog(`  ${hexStr}`, 'env-info');
        if (evt.response) {
            rxLog(`  RESPONSE ${hex(evt.response)}`, 'env-info');
        }
        projectEnvelope(evt.envelope, evt.bitboard);
        updateMeshDisplay(evt.envelope);
        if (isAudioEnvelope(evt.envelope)) {
            const samples = unpackAudioSamples(evt.envelope);
            state.rxAudioQueue.push(samples);
            rxLog(`  AUDIO: ${samples.length} samples queued (total ${state.rxAudioQueue.reduce((a,f)=>a+f.length,0)})`, 'env-info');
            playAudioQueue();
        }
        const frameData = viewerBridge.pushEnvelope(evt.envelope);
        updateViewerStats(frameData);
    };

    btnConnect.addEventListener('click', async () => {
        const baud = parseInt(document.getElementById('baud-rate').value);
        const ok = await state.serial.connect(baud);
        if (ok) {
            setStatus('status-serial', true);
            btnConnect.disabled = true;
            btnDisconnect.disabled = false;
            updateInjectButtons(true);
            log('info', 'Serial connected at ' + baud + ' baud');
            startMeshDebug();
        }
    });

    btnDisconnect.addEventListener('click', async () => {
        stopMeshDebug();
        await state.serial.disconnect();
        setStatus('status-serial', false);
        btnConnect.disabled = false;
        btnDisconnect.disabled = true;
        updateInjectButtons(false);
        log('info', 'Disconnected');
    });

    btnSensor.addEventListener('click', () => {
        const pin = parseInt(document.getElementById('inject-pin').value);
        const value = parseInt(document.getElementById('inject-value').value);
        Module._omi_web_inject_sensor(pin, value);
        rxLog(`INJECT sensor pin=${pin} value=${value}`);
        if (Module._omi_web_has_event()) {
            state.serial.handleEnvelope();
        }
    });

    btnEvent.addEventListener('click', () => {
        const type = document.getElementById('inject-event-type').value;
        const value = parseInt(document.getElementById('inject-event-value').value);
        Module._omi_web_inject_event(type, value);
        rxLog(`INJECT event type=${type} value=${value}`);
        if (Module._omi_web_has_event()) {
            state.serial.handleEnvelope();
        }
    });

    btnHardware.addEventListener('click', () => {
        const device = document.getElementById('inject-device').value;
        const action = document.getElementById('inject-action').value;
        Module._omi_web_inject_hardware(device, action);
        rxLog(`INJECT hardware device=${device} action=${action}`);
        if (Module._omi_web_has_event()) {
            state.serial.handleEnvelope();
        }
    });

    state.audio = new OMIWebAudio();
    document.getElementById('sample-rate').addEventListener('change', () => {
        // rate change takes effect on next init
    });

    btnRecord.addEventListener('click', async () => {
        try {
            const sampleRate = parseInt(document.getElementById('sample-rate').value);
            await state.audio.init(sampleRate);
            state.audio.onFrame = (samples) => {
                state.recordedFrames.push(samples);
                const peak = Math.max(...samples.map(Math.abs)) || 1;
                const pct = Math.min(100, (peak / 32768) * 100);
                document.getElementById('vu-bar').style.width = pct + '%';

                const env = new Uint8Array(64);
                const preheader = [0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF];
                env.set(preheader, 0);
                env[8] = 0x0A; env[9] = 0x01;
                for (let i = 0; i < 16 && i < samples.length; i++) {
                    const s = samples[i];
                    env[32 + i * 2] = (s >> 8) & 0xFF;
                    env[32 + i * 2 + 1] = s & 0xFF;
                }
                if (state.serial && state.serial.connected) {
                    state.serial.sendEnvelope(env);
                }
                document.getElementById('audio-log').textContent =
                    `Frames: ${state.recordedFrames.length}  Samples: ${state.recordedFrames.reduce((a,f) => a+f.length, 0)}`;
            };
            await state.audio.startRecording();
            setStatus('status-audio', true);
            btnRecord.disabled = true;
            btnStop.disabled = false;
            log('info', 'Audio recording started');
        } catch (err) {
            log('error', 'Audio start failed: ' + err.message);
        }
    });

    btnStop.addEventListener('click', () => {
        state.audio.stopRecording();
        setStatus('status-audio', false);
        btnRecord.disabled = false;
        btnStop.disabled = true;
        document.getElementById('vu-bar').style.width = '0%';
        log('info', 'Audio stopped');
    });

    document.getElementById('btn-mesh-refresh').addEventListener('click', requestMeshStatus);

    document.getElementById('btn-toggle-viewer').addEventListener('click', toggleViewer);
    document.getElementById('btn-close-viewer').addEventListener('click', toggleViewer);

    log('info', 'OMI Web Bridge ready');
}

document.addEventListener('DOMContentLoaded', init);
