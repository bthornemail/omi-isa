/* ViewerBridge — envelope -> 3D geometric projection
   Incidence-first: validate finite relation before projection.
   Faces resolved at projection boundary. Constants derived as witnesses. */

class ViewerBridge {
    constructor() {
        this.receipts = [];
        this.frameCounts = { US: 0, GS: 0, RS: 0, FS: 0 };
        this.hopfFlux = [0, 0, 0];
        this.cycle = 0;
        this.maxReceipts = 500;
        this.twinViewer = null;
        this.lazyInitDone = false;
        this.facePanel = null;
        this.lastFaces = [];
        this.lastConstants = null;
        this.lastEnv = null;
    }

    setTwinViewer(tv) {
        this.twinViewer = tv;
    }

    initViewer(container) {
        if (this.lazyInitDone) return;
        try {
            const tv = new TwinViewer(container);
            this.setTwinViewer(tv);
            tv.animate();
            this.lazyInitDone = true;
        } catch (e) {
            console.warn('TwinViewer init:', e.message);
        }
    }

    setFacePanel(panel) {
        this.facePanel = panel;
    }

    /* ── Incidence validation gate ──
       Checks canonical pre-header: [0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF]
       gauge[0] must be >= 0xF0 for valid envelope. */
    validateIncidence(env) {
        if (!env || env.length < 16) return false;
        if (env[0] !== 0xFF) return false;
        if (env[1] !== 0x00) return false;
        if (env[2] !== 0x1C) return false;
        if (env[3] !== 0x1D) return false;
        if (env[4] !== 0x1E) return false;
        if (env[5] !== 0x1F) return false;
        if (env[6] !== 0x20) return false;
        if (env[7] !== 0xFF) return false;
        return true;
    }

    /* Gauge action code from gauge[0] low bits */
    gaugeAction(env) {
        return env ? (env[0] & 0x1F) : 0;
    }

    /* ── Parameter extraction (mirrors omicron_resolve in C) ── */
    extractHopfParams(env) {
        const chart11 = ((env[0] ^ env[8]) + env[4]) % 11;
        const baseQ  = env[1] & 0x03;
        const fiberQ = env[2] & 0x03;
        const fano7  = env[3] % 7;
        const role3  = (env[4] + env[5]) % 3;
        return { chart11, baseQ, fiberQ, fano7, role3 };
    }

    processEnvelope(env) {
        this.lastEnv = env;

        /* 1. Validate incidence first */
        const valid = this.validateIncidence(env);

        /* 2. Gauge action */
        const action = this.gaugeAction(env);

        /* 3. Resolve faces at projection boundary */
        this.lastFaces = resolveFaces(env, action);
        const primaryFace = this.lastFaces.length > 0 ? this.lastFaces[0].name : 'Omi-Form';

        /* 4. Extract Hopf params and compute geometry */
        const p = this.extractHopfParams(env);
        const route = resolveHopfQuquartRoute(p.chart11, p.baseQ, p.fiberQ, p.fano7, p.role3);

        const frameNames = ['US','GS','RS','FS'];
        const frame = frameNames[route.frameType] || 'US';
        this.frameCounts[frame] = (this.frameCounts[frame] || 0) + 1;

        const c = route.c || 0.001;
        const denom = 1 + Math.abs(c);
        const gr = route.a / denom;
        const gi = route.b / denom;
        const rho = Math.sqrt(gr*gr + gi*gi);
        const theta = Math.atan2(gi, gr);
        const d = (1 - gr)*(1 - gr) + gi*gi || 0.001;
        const zr = (1 - gr*gr - gi*gi) / d;
        const zi = (2 * gi) / d;
        const zmag = zr*zr + zi*zi || 0.001;
        const yr = zr / zmag;
        const yi = -zi / zmag;

        this.hopfFlux = [
            this.hopfFlux[0] + route.a,
            this.hopfFlux[1] + route.b,
            this.hopfFlux[2] + route.c
        ];

        /* 5. Derive constants at projection boundary */
        this.lastConstants = deriveConstants(route.polybiusRow, route.polybiusCol);

        /* 6. Build receipt with face info + constants */
        const receipt = {
            cy: this.receipts.length,
            slot: route.slot5040,
            valid: valid,
            gaugeAction: action,
            primaryFace: primaryFace,
            faces: this.lastFaces.map(f => f.name),
            twin: {
                slot: route.slot5040,
                frame: frame,
                hopf: [route.a, route.b, route.c],
                cell: [route.polybiusRow, route.polybiusCol],
                smith: {
                    gamma: [gr, gi],
                    z: [zr, zi],
                    y: [yr, yi],
                    rho: rho,
                    theta: theta
                }
            },
            constants: this.lastConstants,
        };

        this.receipts.unshift(receipt);
        if (this.receipts.length > this.maxReceipts) this.receipts.length = this.maxReceipts;

        this.cycle++;
        return this.buildFrameData();
    }

    buildFrameData() {
        return {
            twin: {
                cycle: this.cycle,
                ring: { size: 5040, xor: '0000', sum: '0000', rot: '0000' },
                summary: {
                    filled: Math.min(this.receipts.length, 5040),
                    frame_counts: [
                        { name: 'US', count: this.frameCounts.US },
                        { name: 'GS', count: this.frameCounts.GS },
                        { name: 'RS', count: this.frameCounts.RS },
                        { name: 'FS', count: this.frameCounts.FS }
                    ],
                    hopf_flux: [
                        this.hopfFlux[0],
                        this.hopfFlux[1],
                        this.hopfFlux[2]
                    ]
                },
                receipts: this.receipts.slice(0, 200),
                faces: this.lastFaces,
                constants: this.lastConstants,
            }
        };
    }

    pushEnvelope(env) {
        const frameData = this.processEnvelope(env);
        if (this.twinViewer) {
            this.twinViewer.update(frameData);
        }
        if (this.facePanel) {
            this.facePanel.update(this.lastEnv, this.lastFaces, this.lastConstants);
        }
    }
}
