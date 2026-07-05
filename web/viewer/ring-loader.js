/* Ring loader -- fetches deterministic state from omicron.bin --serve */

class RingLoader {
  constructor(baseUrl) {
    this.baseUrl = baseUrl || '';
    this.lastFrame = null;
    this.listeners = [];
    this._sseSource = null;
    this._ws = null;
    this._polling = false;
  }

  onUpdate(fn) { this.listeners.push(fn); }

  _emit(frame) {
    if (JSON.stringify(frame) !== JSON.stringify(this.lastFrame)) {
      this.lastFrame = frame;
      for (const fn of this.listeners) fn(frame);
    }
  }

  async fetchFrame() {
    const res = await fetch(this.baseUrl + '/frame');
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return await res.json();
  }

  async fetchRing() {
    const res = await fetch(this.baseUrl + '/ring');
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return await res.json();
  }

  async pollFrame(intervalMs) {
    this._polling = true;
    while (this._polling) {
      try {
        const frame = await this.fetchFrame();
        this._emit(frame);
      } catch (e) {
        console.error('Poll error:', e);
      }
      await new Promise(r => setTimeout(r, intervalMs || 1000));
    }
  }

  stopPolling() { this._polling = false; }

  /* ─── SSE streaming ─── */
  connectSSE() {
    this.disconnect();
    const url = this.baseUrl + '/events';
    this._sseSource = new EventSource(url);
    this._sseSource.onmessage = (e) => {
      try { this._emit(JSON.parse(e.data)); } catch (err) { console.error('SSE parse error:', err); }
    };
    this._sseSource.onerror = () => {
      console.warn('SSE connection failed, falling back to polling');
      this._sseSource = null;
      this.pollFrame(2000);
    };
  }

  /* ─── WebSocket streaming ─── */
  connectWS() {
    this.disconnect();
    const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
    const url = proto + '//' + location.host + this.baseUrl + '/ws';
    this._ws = new WebSocket(url);
    this._ws.onmessage = (e) => {
      try { this._emit(JSON.parse(e.data)); } catch (err) { console.error('WS parse error:', err); }
    };
    this._ws.onclose = () => {
      console.warn('WebSocket closed, falling back to SSE');
      this._ws = null;
      this.connectSSE();
    };
    this._ws.onerror = () => {
      console.warn('WebSocket error, falling back to SSE');
      this._ws.close();
      this._ws = null;
      this.connectSSE();
    };
  }

  /* ─── Auto-detect: WS → SSE → polling ─── */
  connect() {
    this.connectWS();
  }

  disconnect() {
    this.stopPolling();
    if (this._sseSource) { this._sseSource.close(); this._sseSource = null; }
    if (this._ws) { this._ws.close(); this._ws = null; }
  }
}

/* Total time: 0.037s */
