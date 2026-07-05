/* Face Registry — named surface projection at the projection boundary
   Maps: (name . face) -> (face . role) -> (role . edge) -> (edge . receipt-path)
   Only validation + receipt accepts the resolved relation. */

const FACE_CATEGORIES = {
    core:       'Core',
    authority:  'Authority / Witness',
    scope:      'Scope / Tower',
    runtime:    'Runtime / Process',
    function:   'Function / Computation',
    geometry:   'Geometry / Configuration',
    carrier:    'Carrier / Encoding',
    hardware:   'Hardware / Sensory / Body',
    graph:      'Graph / Node Process',
};

const FACE_DB = {

    /* ── Core Faces ── */
    'Omi-Form':        { cat: 'core',   role: 'structural',             edge: 'relation.structure' },
    'Omi-Glyph':       { cat: 'core',   role: 'symbolic',               edge: 'relation.symbol' },
    'Omi-Hash':        { cat: 'core',   role: 'digest',                 edge: 'relation.digest' },
    'Omi-Map':         { cat: 'core',   role: 'coordinate',             edge: 'relation.route' },
    'Omi-Image':       { cat: 'core',   role: 'recoverable',            edge: 'relation.package' },
    'Omi-Matrix':      { cat: 'core',   role: 'observation',            edge: 'relation.observe' },
    'Omi-Gnomon':      { cat: 'core',   role: 'orientation',            edge: 'relation.bearing' },
    'Omi-Mirror':      { cat: 'core',   role: 'chirality',              edge: 'relation.inverse' },
    'Omi-Clock':       { cat: 'core',   role: 'timing',                 edge: 'relation.cadence' },
    'Omi-Portal':      { cat: 'core',   role: 'access',                 edge: 'relation.doorway' },
    'Omi-Sense':       { cat: 'core',   role: 'sensory reader',         edge: 'transducer' },
    'Omi-World':       { cat: 'core',   role: 'persistent environment', edge: 'world' },
    'Omi-Environment': { cat: 'core',   role: 'world state',            edge: 'ambient' },

    /* ── Authority / Witness Faces ── */
    'Omi-Receipt':  { cat: 'authority', role: 'witness',      edge: 'relation.resolution' },
    'Omi-Gate':     { cat: 'authority', role: 'origin',       edge: 'relation.zero' },
    'Omi-Genesis':  { cat: 'authority', role: 'first-cons',   edge: 'relation.birth' },
    'Omi-Point':    { cat: 'authority', role: 'atom',         edge: 'relation.smallest' },
    'Omi-Shadow':   { cat: 'authority', role: 'secondary',    edge: 'relation.projection' },

    /* ── Scope / Tower Faces ── */
    'Omi-FS':   { cat: 'scope', role: 'file-frame',     edge: 'relation.spatial' },
    'Imo-FS':   { cat: 'scope', role: 'file-carrier',   edge: 'relation.anchor' },
    'Omi-GS':   { cat: 'scope', role: 'graph-guide',    edge: 'relation.layer' },
    'Imo-GS':   { cat: 'scope', role: 'graph-carrier',  edge: 'relation.guidance' },
    'Omi-RS':   { cat: 'scope', role: 'record-witness', edge: 'relation.layer' },
    'Imo-RS':   { cat: 'scope', role: 'record-carrier', edge: 'relation.resolution' },
    'Omi-US':   { cat: 'scope', role: 'unit-memory',    edge: 'relation.layer' },
    'Imo-US':   { cat: 'scope', role: 'unit-carrier',   edge: 'relation.understanding' },
    'Omi-Node': { cat: 'scope', role: 'node-face',      edge: 'relation.dom' },
    'Omi-Frame':{ cat: 'scope', role: 'carrier-frame',  edge: 'relation.projection' },

    /* ── Runtime / Process Faces ── */
    'Omi-Automaton':  { cat: 'runtime', role: 'agreement-machine', edge: 'relation.automaton' },
    'Omi-Actor':      { cat: 'runtime', role: 'supervised-cell',   edge: 'relation.cell' },
    'Omi-Observer':   { cat: 'runtime', role: 'witness-client',    edge: 'relation.reader' },
    'Omi-Bus':        { cat: 'runtime', role: 'routing-surface',   edge: 'relation.service' },
    'Omni-Router':    { cat: 'runtime', role: 'dispatch-surface',  edge: 'relation.ingestion' },
    'Omi-Process':    { cat: 'runtime', role: 'process-envelope',  edge: 'relation.process' },
    'Omi-Trace':      { cat: 'runtime', role: 'replay-log',        edge: 'relation.ordered' },
    'Omi-Lisp':       { cat: 'runtime', role: 'computation-layer', edge: 'relation.dot-notation' },
    'Omi-Alist':      { cat: 'runtime', role: 'declaration-face',  edge: 'relation.association' },

    /* ── Function / Computation Faces ── */
    'Omi-Gauge':      { cat: 'function', role: 'spatial-resolver',     edge: 'relation.gauge' },
    'Omi-Nomogram':   { cat: 'function', role: 'function-selector',    edge: 'relation.scale' },
    'Omi-SlideRule':  { cat: 'function', role: 'scale-walk',           edge: 'relation.operational' },
    'Omi-Query':      { cat: 'function', role: 'payload-plane',        edge: 'relation.post-address' },
    'Omi-CONS':       { cat: 'function', role: 'binding-frame',        edge: 'relation.payload' },
    'Omi-CAR':        { cat: 'function', role: 'head-view',            edge: 'relation.source' },
    'Omi-CDR':        { cat: 'function', role: 'tail-view',            edge: 'relation.continuation' },
    'Omi-CID':        { cat: 'function', role: 'agreement-view',       edge: 'relation.witness-view' },
    'Omi-CONS256':    { cat: 'function', role: 'symbolic-meta-object', edge: 'relation.meta' },
    'Omi-Notation':   { cat: 'function', role: 'tuple-notation',       edge: 'relation.streamable' },

    /* ── Geometry / Configuration Faces ── */
    'Omi-Ring':     { cat: 'geometry', role: 'circular-bridge',  edge: 'relation.spectral' },
    'Omi-Compass':  { cat: 'geometry', role: 'agreement-bearing',edge: 'relation.orientation' },
    'Omi-Torus':    { cat: 'geometry', role: 'gray-code-surface',edge: 'relation.karnaugh' },
    'Omi-Chart':    { cat: 'geometry', role: 'scalar-chart',     edge: 'relation.smith' },
    'Omi-Voxel':    { cat: 'geometry', role: 'tile-map',         edge: 'relation.extrusion' },
    'Omi-Dali':     { cat: 'geometry', role: 'hypercube-unfold', edge: 'relation.subsurface' },
    'Omi-Shader':   { cat: 'geometry', role: 'gpu-projection',   edge: 'relation.acceleration' },
    'Omi-Mesh':     { cat: 'geometry', role: 'field-network',    edge: 'relation.located' },
    'Omi-NEAT':     { cat: 'geometry', role: 'entropy-topology', edge: 'relation.layer' },
    'Omi-Psi':      { cat: 'geometry', role: 'sensed-field',     edge: 'relation.unified' },
    'Omi-Light':    { cat: 'geometry', role: 'visual emission',  edge: 'photon' },
    'Omi-Sound':    { cat: 'geometry', role: 'sound/cadence',    edge: 'acoustic' },
    'Omi-Motion':   { cat: 'geometry', role: 'movement',         edge: 'velocity' },

    /* ── Carrier / Encoding Faces ── */
    'Omi-Tape':       { cat: 'carrier', role: 'sequential-carrier',  edge: 'relation.script' },
    'Omi-Barcode':    { cat: 'carrier', role: 'barcode-family',      edge: 'relation.canonical' },
    'Omi-JabCode':    { cat: 'carrier', role: 'polychrome-matrix',   edge: 'relation.matrix' },
    'Omi-Bidi':       { cat: 'carrier', role: 'chirality-steering',  edge: 'relation.direction' },
    'Omi-Bitboard':   { cat: 'carrier', role: 'dense-state',         edge: 'relation.compatibility' },
    'Omi-RewriteTable':{ cat: 'carrier', role: 'transition-table',   edge: 'relation.deterministic' },
    'Omi-Macro':      { cat: 'carrier', role: 'rewrite-declaration', edge: 'relation.source-level' },
    'Omi-Script':     { cat: 'carrier', role: 'executable-projection',edge: 'relation.runtime' },

    /* ── Hardware / Sensory / Body Faces ── */
    'Omi-Pyramid':      { cat: 'hardware', role: 'encoder-automaton',   edge: 'relation.encode' },
    'Omi-Amulet':       { cat: 'hardware', role: 'decoder-automaton',   edge: 'relation.decode' },
    'Omi-Dome':         { cat: 'hardware', role: 'immersive-world',     edge: 'relation.sensory' },
    'Omi-Base':         { cat: 'hardware', role: 'rotational-platform', edge: 'relation.sync' },
    'Omi-Radio':        { cat: 'hardware', role: 'gossip-carrier',      edge: 'relation.rf' },
    'OMI-GPIO':         { cat: 'hardware', role: 'voltage-transducer',  edge: 'relation.gpio' },
    'Omi-Talisman':     { cat: 'hardware', role: 'meaning-mirror',      edge: 'relation.personal' },
    'Omi-Avatar':       { cat: 'hardware', role: 'digital-body',        edge: 'relation.projection' },
    'Omi-Skeleton':     { cat: 'hardware', role: 'semantic-rig',        edge: 'relation.structure' },
    'Omi-Touch':        { cat: 'hardware', role: 'haptic/contact',      edge: 'pressure' },
    'Omi-Gesture':      { cat: 'hardware', role: 'motion/pattern',      edge: 'movement' },
    'Omi-Temperature':  { cat: 'hardware', role: 'thermal',             edge: 'heat' },
    'Omi-Proximity':    { cat: 'hardware', role: 'near/far',            edge: 'distance' },
    'SpectrumDOM':      { cat: 'hardware', role: 'wavelength-matrix',   edge: 'relation.field' },

    /* ── Graph / Node Process Faces ── */
    'Omi-Graph':        { cat: 'graph', role: 'class-container',    edge: 'relation.class' },
    'Omi-Edge':         { cat: 'graph', role: 'relation-io',        edge: 'relation.input' },
    'Omi-NodeGraph':    { cat: 'graph', role: 'process-topology',   edge: 'relation.topology' },
    'Omi-NodeFunction': { cat: 'graph', role: 'pure-transition',    edge: 'relation.edge' },
};

/* Gauge action -> primary face mapping */
const GAUGE_TO_FACE = {
    0x00: 'Omi-Form',
    0x1C: 'Omi-Glyph',
    0x1D: 'Omi-Glyph',
    0x1E: 'Omi-Map',
    0x1F: 'Omi-Map',
    0x20: 'Omi-Matrix',
};

/* Orientation marker -> secondary face mapping */
const ORIENTATION_TO_FACE = {
    0x01: 'Omi-Mesh',
    0x02: 'Omi-Route',
    0x03: 'Omi-Gate',
    0x04: 'Omi-Mesh',
    0x05: 'Omi-Mesh',
    0x0A: 'Omi-Sound',
    0x0B: 'Omi-Light',
    0x0C: 'Omi-Sense',
};

function faceCategory(name) {
    const f = FACE_DB[name];
    return f ? f.cat : null;
}

function faceRole(name) {
    const f = FACE_DB[name];
    return f ? f.role : null;
}

function faceEdge(name) {
    const f = FACE_DB[name];
    return f ? f.edge : null;
}

function faceReceiptPath(name) {
    const role = faceRole(name);
    if (!role) return null;
    return 'accepted.' + role;
}

/* Resolve primary face from gauge action code */
function resolveGaugeFace(gaugeAction) {
    const name = GAUGE_TO_FACE[gaugeAction];
    if (name) return name;
    if (gaugeAction >= 0xF0) return 'Omi-Receipt';
    return 'Omi-Form';
}

/* Resolve secondary faces from orientation[0] */
function resolveOrientationFaces(orientByte) {
    const name = ORIENTATION_TO_FACE[orientByte];
    return name ? [name] : [];
}

/* Resolve tertiary faces from gauge table activity */
function resolveGaugeFaces(env) {
    const faces = [];
    if (env[0] >= 0xF0) faces.push('Omi-Receipt');
    if (env[0] === 0x00) faces.push('Omi-Genesis');
    if (env[4] !== 0 || env[5] !== 0) faces.push('Omi-Map');
    if (env[6] !== 0 || env[7] !== 0) faces.push('Omi-Gnomon');
    return faces;
}

/* Complete face resolution for an envelope */
function resolveFaces(env, gaugeAction) {
    const primary = resolveGaugeFace(gaugeAction);
    const secondary = resolveOrientationFaces(env[8]);
    const tertiary = resolveGaugeFaces(env);

    const all = [primary];
    for (const f of secondary) if (all.indexOf(f) < 0) all.push(f);
    for (const f of tertiary) if (all.indexOf(f) < 0) all.push(f);

    return all.map(name => {
        const entry = FACE_DB[name] || { cat: 'core', role: 'unknown', edge: 'relation.unknown' };
        return {
            name: name,
            category: FACE_CATEGORIES[entry.cat] || 'Core',
            catKey: entry.cat || 'core',
            role: entry.role,
            edge: entry.edge,
            receiptPath: 'accepted.' + entry.role,
        };
    });
}

/* Build a reduction chain for display */
function faceReductionChain(face) {
    return [
        { step: 'name', value: face.name },
        { step: 'face', value: '(' + face.name.replace('Omi-', '') + ' . ' + face.role + ')' },
        { step: 'role', value: '(' + face.role + ' . ' + face.edge.replace('relation.', '') + ')' },
        { step: 'edge', value: '(' + face.edge.replace('relation.', '') + ' . ' + face.receiptPath.replace('accepted.', '') + ')' },
        { step: 'receipt', value: face.receiptPath },
    ];
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        FACE_DB, FACE_CATEGORIES, GAUGE_TO_FACE, ORIENTATION_TO_FACE,
        faceCategory, faceRole, faceEdge, faceReceiptPath,
        resolveGaugeFace, resolveOrientationFaces, resolveGaugeFaces,
        resolveFaces, faceReductionChain,
    };
}
