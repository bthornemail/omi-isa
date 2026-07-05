class OmiAudioProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.buffer = [];
        this.frameSize = 16;
        this.port.onmessage = (e) => {
            if (e.data && e.data.frameSize) this.frameSize = e.data.frameSize;
        };
    }

    process(inputs) {
        const input = inputs[0];
        if (!input || !input[0]) return true;
        const samples = input[0];
        for (const s of samples) {
            this.buffer.push(Math.max(-32768, Math.min(32767, Math.round(s * 32768))));
            if (this.buffer.length >= this.frameSize) {
                this.port.postMessage(new Int16Array(this.buffer));
                this.buffer = [];
            }
        }
        return true;
    }
}

registerProcessor('omi-audio-processor', OmiAudioProcessor);
