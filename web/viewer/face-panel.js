/* Face Panel — displays resolved face(s), reduction chain, constant witnesses */

class FacePanel {
    constructor(containerId) {
        this.container = document.getElementById(containerId);
        this.currentFaces = [];
        this.currentConstants = null;
    }

    update(env, faces, constants) {
        this.currentFaces = faces || [];
        this.currentConstants = constants || null;
        this.render();
    }

    render() {
        if (!this.container) return;
        this.container.innerHTML = '';

        const faces = this.currentFaces;
        if (!faces.length) {
            this.container.innerHTML = '<div class="face-empty">No faces resolved</div>';
            return;
        }

        /* Face cards */
        for (const face of faces) {
            const card = document.createElement('div');
            card.className = 'face-card face-cat-' + (face.catKey || 'core');
            card.innerHTML = this.faceCardHTML(face);
            this.container.appendChild(card);

            /* Reduction chain */
            const chain = faceReductionChain(face);
            const chainEl = document.createElement('div');
            chainEl.className = 'face-chain';
            chainEl.innerHTML = chain.map((s, i) =>
                '<span class="chain-step chain-step-' + s.step + '">' +
                (i > 0 ? ' \u2192 ' : '') +
                '<span class="chain-label">' + s.step + '</span> ' +
                '<span class="chain-value">' + s.value + '</span></span>'
            ).join('');
            this.container.appendChild(chainEl);
        }

        /* Constant witnesses */
        if (this.currentConstants) {
            this.renderConstants();
        }
    }

    faceCardHTML(face) {
        const catColors = {
            core: '#58a6ff',
            authority: '#3fb950',
            scope: '#d29922',
            runtime: '#f85149',
            function: '#bc8cff',
            geometry: '#79c0ff',
            carrier: '#ffa657',
            hardware: '#ff7b72',
            graph: '#3fb950',
        };
        const color = catColors[face.catKey] || '#8b949e';
        return '<div class="face-card-header" style="border-left-color:' + color + '">' +
            '<span class="face-name" style="color:' + color + '">' + face.name + '</span>' +
            '<span class="face-cat-badge badge-' + face.catKey + '">' + face.category + '</span>' +
            '</div>' +
            '<div class="face-card-body">' +
            '<div class="face-role"><span class="face-label">role</span> ' + face.role + '</div>' +
            '<div class="face-edge"><span class="face-label">edge</span> ' + face.edge + '</div>' +
            '<div class="face-receipt"><span class="face-label">receipt</span> ' + face.receiptPath + '</div>' +
            '</div>';
    }

    renderConstants() {
        const c = this.currentConstants;
        const block = document.createElement('div');
        block.className = 'constants-block';
        block.innerHTML =
            '<div class="constants-title">\u03C0 \u221A3 \u03C6 Projection Witnesses</div>' +
            '<div class="constants-body">' +
            this.constRow('\u03C0', c.pi.value.toFixed(10), c.pi.lane, c.pi.checks) +
            this.constRow('\u221A3', c.sqrt3.value.toFixed(10), c.sqrt3.lane, c.sqrt3.checks) +
            this.constRow('\u03C6', c.phi.value.toFixed(10), c.phi.lane, c.phi.checks, c.phi.convergence) +
            '</div>';
        this.container.appendChild(block);
    }

    constRow(symbol, value, lane, checks, convergence) {
        const convBadge = convergence
            ? (convergence.converged
                ? '<span class="conv-badge converged">converged \u2713</span>'
                : '<span class="conv-badge diverged">\u0394=' + convergence.delta.toExponential(2) + '</span>')
            : '';
        return '<div class="const-row">' +
            '<div class="const-symbol">' + symbol + '</div>' +
            '<div class="const-value">' + value + '</div>' +
            '<div class="const-lane">' + lane + '</div>' +
            convBadge +
            '<div class="const-checks">' + checks.map(c => '<span class="check-item">' + c + '</span>').join('') + '</div>' +
            '</div>';
    }
}
