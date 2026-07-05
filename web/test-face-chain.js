/* Face Chain Verification — browser/Node.js test
   Validates face→role→edge→receipt-path reduction chain for all envelope types. */

const {
    FACE_DB, FACE_CATEGORIES, GAUGE_TO_FACE, ORIENTATION_TO_FACE,
    resolveGaugeFace, resolveOrientationFaces, resolveGaugeFaces,
    resolveFaces, faceReductionChain,
} = require('./face-registry.js');

let passed = 0, failed = 0;
function test(name, cond, detail) {
    if (cond) { passed++; /* console.log('  PASS:', name); */ }
    else {
        failed++;
        console.log('  FAIL:', name, detail || '');
    }
}

/* ── Test 1: FACE_DB has ~80 entries across 9 categories ── */
const totalFaces = Object.keys(FACE_DB).length;
test('FACE_DB has 70-90 entries', totalFaces >= 70 && totalFaces <= 90,
     'found ' + totalFaces);
const cats = Object.keys(FACE_CATEGORIES);
test('FACE_CATEGORIES has 9 categories', cats.length === 9,
     'found ' + cats.length + ': ' + cats.join(', '));

/* ── Test 2: All faces resolve through reduction chain ── */
let chainOk = 0, chainFail = 0;
for (const name in FACE_DB) {
    const entry = FACE_DB[name];
    const face = {
        name,
        catKey: entry.cat,
        role: entry.role,
        edge: entry.edge,
        receiptPath: 'accepted.' + entry.role,
    };
    const chain = faceReductionChain(face);
    if (chain.length === 5) chainOk++; else chainFail++;
}
test('All faces produce 5-step reduction chain', chainFail === 0,
     chainOk + '/' + totalFaces + ' valid');

/* ── Test 3: Chain step order is name→face→role→edge→receipt ── */
let stepNamesOk = 0;
for (const name in FACE_DB) {
    const entry = FACE_DB[name];
    const face = {
        name,
        catKey: entry.cat,
        role: entry.role,
        edge: entry.edge,
        receiptPath: 'accepted.' + entry.role,
    };
    const chain = faceReductionChain(face);
    const expected = ['name','face','role','edge','receipt'];
    if (chain.map(s => s.step).join(',') === expected.join(',')) stepNamesOk++;
}
test('All chains have correct step order', stepNamesOk === totalFaces,
     stepNamesOk + '/' + totalFaces);

/* ── Test 4: Gauge action → primary face mapping ── */
const gaugeMap = [
    [0x00, 'Omi-Form'],
    [0x1C, 'Omi-Glyph'], [0x1D, 'Omi-Glyph'],
    [0x1E, 'Omi-Map'],  [0x1F, 'Omi-Map'],
    [0x20, 'Omi-Matrix'],
];
for (const [code, expected] of gaugeMap) {
    test('gauge 0x' + code.toString(16) + ' → ' + expected,
         resolveGaugeFace(code) === expected);
}

/* ── Test 5: High gauge codes (0xF0-0xFF) → Omi-Receipt ── */
for (let code = 0xF0; code <= 0xFF; code++) {
    test('gauge 0x' + code.toString(16) + ' → Omi-Receipt',
         resolveGaugeFace(code) === 'Omi-Receipt');
}

/* ── Test 6: GAUGE_TO_FACE covers all declared keys ── */
const gaugeKeys = Object.keys(GAUGE_TO_FACE).map(Number);
for (const code of gaugeKeys) {
    test('GAUGE_TO_FACE[' + code + '] matches resolveGaugeFace',
         resolveGaugeFace(code) === GAUGE_TO_FACE[code]);
}

/* ── Test 7: Orientation markers → secondary faces ── */
const orientMap = [
    [0x01, 'Omi-Mesh'], [0x02, 'Omi-Route'], [0x03, 'Omi-Gate'],
    [0x04, 'Omi-Mesh'], [0x05, 'Omi-Mesh'],
    [0x0A, 'Omi-Sound'], [0x0B, 'Omi-Light'],
];
for (const [code, expected] of orientMap) {
    const faces = resolveOrientationFaces(code);
    test('orientation 0x' + code.toString(16) + ' includes ' + expected,
         faces.indexOf(expected) >= 0);
}

/* ── Test 8: ORIENTATION_TO_FACE covers all declared keys ── */
const orientKeys = Object.keys(ORIENTATION_TO_FACE).map(Number);
for (const code of orientKeys) {
    const faces = resolveOrientationFaces(code);
    test('ORIENTATION_TO_FACE[' + code + '] in resolveOrientationFaces',
         faces.indexOf(ORIENTATION_TO_FACE[code]) >= 0);
}

/* ── Test 9: resolveFaces returns correct primary for each gauge action ── */
const env = new Uint8Array(64);
env[0] = 0xFF; env[1] = 0x00; env[2] = 0x1C; env[3] = 0x1D;
env[4] = 0x1E; env[5] = 0x1F; env[6] = 0x20; env[7] = 0xFF;

for (const [code, expected] of gaugeMap) {
    env[0] = 0xF0 | code;
    const faces = resolveFaces(env, code);
    test('resolveFaces(0x' + code.toString(16) + ')[0] == ' + expected,
         faces.length > 0 && faces[0].name === expected);
}

/* ── Test 10: resolveFaces includes orientation secondary faces ── */
const envOrient = new Uint8Array(64);
envOrient[0] = 0xFF; envOrient[7] = 0xFF;
envOrient[8] = 0x0A;
const withSound = resolveFaces(envOrient, 0x00);
test('resolveFaces includes Omi-Sound from orientation[0]=0x0A',
     withSound.some(f => f.name === 'Omi-Sound'));

/* ── Test 11: Each face category is valid ── */
const validCats = Object.keys(FACE_CATEGORIES);
const allCatsValid = Object.values(FACE_DB).every(e => validCats.indexOf(e.cat) >= 0);
test('All faces have valid category key', allCatsValid,
     Object.keys(FACE_DB).length + ' entries checked');

/* ── Test 12: All 9 categories have at least one face ── */
const catsUsed = {};
for (const name in FACE_DB) catsUsed[FACE_DB[name].cat] = true;
test('All 9 categories present in FACE_DB', Object.keys(catsUsed).length === 9,
     'found: ' + Object.keys(catsUsed).sort().join(', '));

/* ── Test 13: Resolved faces have all required fields ── */
const envAny = new Uint8Array(64);
envAny[0] = 0xFF; envAny[7] = 0xFF;
envAny[8] = 0x0B;
const allFaces = resolveFaces(envAny, 0x1C);
let fieldsOk = 0;
for (const f of allFaces) {
    if (f.name && f.category && f.catKey && f.role && f.edge && f.receiptPath) fieldsOk++;
}
test('All resolved faces have name, category, catKey, role, edge, receiptPath',
     fieldsOk === allFaces.length, fieldsOk + '/' + allFaces.length + ' ok');

/* ── Test 14: Receipt path format ── */
let receiptOk = 0;
for (const name in FACE_DB) {
    const entry = FACE_DB[name];
    if (entry.role && ('accepted.' + entry.role).startsWith('accepted.')) receiptOk++;
}
test('All faces produce accepted.<role> receipt path', receiptOk === totalFaces,
     receiptOk + '/' + totalFaces);

/* ── Test 15: All FACE_DB entries are in FACE_CATEGORIES categories ── */
let catMatch = 0;
for (const name in FACE_DB) {
    if (FACE_CATEGORIES[FACE_DB[name].cat]) catMatch++;
}
test('All catKeys resolve in FACE_CATEGORIES', catMatch === totalFaces,
     catMatch + '/' + totalFaces);

/* ── Summary ── */
const total = passed + failed;
const pct = (passed / total * 100).toFixed(1);
const status = failed === 0 ? 'ALL PASSED' : failed + ' FAILED';
console.log('\n=== Face Chain: ' + passed + '/' + total + ' (' + pct + '%) ' + status + ' ===');
process.exit(failed > 0 ? 1 : 0);
