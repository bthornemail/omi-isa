class OMIWebAudio {
    constructor() {
        this.ctx = null;
        this.stream = null;
        this.source = null;
        this.worklet = null;
        this.recording = false;
        this.onFrame = null;
    }

    async init(sampleRate) {
        this.ctx = new (window.AudioContext || window.webkitAudioContext)({
            sampleRate: sampleRate || 8000
        });
        await this.ctx.audioWorklet.addModule('omi_audio_worklet.js');
        return this.ctx;
    }

    async startRecording() {
        if (this.recording) return;
        try {
            this.stream = await navigator.mediaDevices.getUserMedia({
                audio: { sampleRate: this.ctx.sampleRate, channelCount: 1,
                         echoCancellation: false, noiseSuppression: false }
            });
            this.source = this.ctx.createMediaStreamSource(this.stream);
            this.worklet = new AudioWorkletNode(this.ctx, 'omi-audio-processor');
            this.worklet.port.onmessage = (e) => {
                if (this.onFrame) this.onFrame(e.data);
            };
            this.source.connect(this.worklet);
            this.recording = true;
        } catch (err) {
            console.error('Audio start failed:', err);
        }
    }

    stopRecording() {
        if (!this.recording) return;
        if (this.worklet) { this.worklet.disconnect(); this.worklet = null; }
        if (this.source) { this.source.disconnect(); this.source = null; }
        if (this.stream) { this.stream.getTracks().forEach(t => t.stop()); this.stream = null; }
        this.recording = false;
    }

    play(samples) {
        if (!this.ctx || !samples.length) return;
        const f32 = new Float32Array(samples.length);
        for (let i = 0; i < samples.length; i++) f32[i] = samples[i] / 32768;
        const buf = this.ctx.createBuffer(1, samples.length, this.ctx.sampleRate);
        buf.copyToChannel(f32, 0);
        const src = this.ctx.createBufferSource();
        src.buffer = buf;
        src.connect(this.ctx.destination);
        src.start();
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { OMIWebAudio };
}
