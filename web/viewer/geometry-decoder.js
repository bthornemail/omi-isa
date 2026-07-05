/* Pure JS TwinGeometry decoder -- deterministic, mirrors omicron.c */

const FANO_LINES = [
  [0,1,2],[0,3,4],[0,5,6],[1,3,5],[1,4,6],[2,3,6],[2,4,5]
];

const QUQUART_PHASE = [0x1f, 0x1d, 0x1e, 0x1c];
const FRAME_NAMES = ['US','GS','RS','FS'];

/* --- Solid geometry (mirrors omicron.c seed shapes) --- */

function tetraGoldenRatio() {
  let x = 2.0;
  for (let i = 0; i < 48; i++) x = 1.0 + 1.0 / x;
  return x;
}
const PHI = tetraGoldenRatio();
const PSI = 1 / PHI;

function v3(x,y,z) { return {x,y,z}; }

function edgesFromPairs(pairs) {
  return pairs.map(p => ({a: p[0], b: p[1]}));
}

const SOLID_DB = {

  /* Tetrahedron: 4 vertices, 6 edges */
  0: {
    name: 'Tetrahedron',
    verts: [v3(1,1,1), v3(1,-1,-1), v3(-1,1,-1), v3(-1,-1,1)],
    edges: edgesFromPairs([[0,1],[0,2],[0,3],[1,2],[1,3],[2,3]])
  },

  /* Stellated Tetrahedron: 8 vertices, 12 edges */
  1: {
    name: 'Stellated Tetrahedron',
    verts: [v3(1,1,1), v3(1,-1,-1), v3(-1,1,-1), v3(-1,-1,1),
            v3(-1,-1,-1), v3(-1,1,1), v3(1,-1,1), v3(1,1,-1)],
    edges: edgesFromPairs([
      [0,1],[0,2],[0,3],[1,2],[1,3],[2,3],
      [4,5],[4,6],[4,7],[5,6],[5,7],[6,7]
    ])
  },

  /* Octahedron: 6 vertices, 12 edges */
  2: {
    name: 'Octahedron',
    verts: [v3(1,0,0), v3(-1,0,0), v3(0,1,0), v3(0,-1,0), v3(0,0,1), v3(0,0,-1)],
    edges: edgesFromPairs([
      [0,2],[0,3],[0,4],[0,5],[1,2],[1,3],[1,4],[1,5],[2,4],[2,5],[3,4],[3,5]
    ])
  },

  /* Cube: 8 vertices, 12 edges */
  3: {
    name: 'Cube',
    verts: [v3(-1,-1,-1), v3(1,-1,-1), v3(1,1,-1), v3(-1,1,-1),
            v3(-1,-1,1), v3(1,-1,1), v3(1,1,1), v3(-1,1,1)],
    edges: edgesFromPairs([
      [0,1],[1,2],[2,3],[3,0],[4,5],[5,6],[6,7],[7,4],
      [0,4],[1,5],[2,6],[3,7]
    ])
  },

  /* Cuboctahedron: 12 vertices, 24 edges */
  4: {
    name: 'Cuboctahedron',
    verts: [v3(1,1,0), v3(1,-1,0), v3(-1,1,0), v3(-1,-1,0),
            v3(1,0,1), v3(1,0,-1), v3(-1,0,1), v3(-1,0,-1),
            v3(0,1,1), v3(0,1,-1), v3(0,-1,1), v3(0,-1,-1)],
    edges: edgesFromPairs([
      [0,4],[0,5],[0,8],[0,9],[1,4],[1,5],[1,10],[1,11],
      [2,6],[2,7],[2,8],[2,9],[3,6],[3,7],[3,10],[3,11],
      [4,8],[4,10],[5,9],[5,11],[6,8],[6,10],[7,9],[7,11]
    ])
  },

  /* Icosahedron: 12 vertices, 30 edges */
  5: {
    name: 'Icosahedron',
    verts: [v3(0,-PHI,1), v3(0,-PHI,-1), v3(0,PHI,1), v3(0,PHI,-1),
            v3(-PHI,1,0), v3(-PHI,-1,0), v3(PHI,1,0), v3(PHI,-1,0),
            v3(1,0,-PHI), v3(-1,0,-PHI), v3(1,0,PHI), v3(-1,0,PHI)],
    edges: edgesFromPairs([
      [0,1],[0,5],[0,7],[0,10],[0,11],
      [1,5],[1,7],[1,8],[1,9],
      [2,3],[2,4],[2,6],[2,10],[2,11],
      [3,4],[3,6],[3,8],[3,9],
      [4,5],[4,9],[4,11],
      [5,9],[5,11],
      [6,7],[6,8],[6,10],
      [7,8],[7,10],
      [8,9],[10,11]
    ])
  },

  /* Dodecahedron: 20 vertices, 30 edges */
  6: {
    name: 'Dodecahedron',
    verts: [v3(-1,-1,-1), v3(1,-1,-1), v3(1,1,-1), v3(-1,1,-1),
            v3(-1,-1,1), v3(1,-1,1), v3(1,1,1), v3(-1,1,1),
            v3(0,-PSI,-PHI), v3(0,PSI,-PHI), v3(0,-PSI,PHI), v3(0,PSI,PHI),
            v3(-PSI,-PHI,0), v3(-PSI,PHI,0), v3(PSI,-PHI,0), v3(PSI,PHI,0),
            v3(-PHI,0,-PSI), v3(-PHI,0,PSI), v3(PHI,0,-PSI), v3(PHI,0,PSI)],
    edges: edgesFromPairs([
      [0,1],[0,2],[0,3],[1,2],[1,5],[2,6],
      [3,7],[4,5],[4,6],[4,7],[5,6],[5,10],
      [6,11],[7,11],[8,9],[8,12],[8,16],[9,13],
      [9,16],[10,11],[10,12],[10,17],[11,13],[11,17],
      [12,14],[12,17],[13,15],[13,17],[14,15],[14,18],
      [15,19],[16,17],[16,18],[18,19]
    ])
  },

  /* Icosidodecahedron: 30 vertices, 60 edges — approximate with truncated data */
  7: {
    name: 'Icosidodecahedron',
    verts: [
      v3(0,0,PHI*2), v3(0,0,-PHI*2),
      v3(PHI*2,0,0), v3(-PHI*2,0,0),
      v3(0,PHI*2,0), v3(0,-PHI*2,0),
      v3(1,1,PHI), v3(1,-1,PHI), v3(-1,1,PHI), v3(-1,-1,PHI),
      v3(1,1,-PHI), v3(1,-1,-PHI), v3(-1,1,-PHI), v3(-1,-1,-PHI),
      v3(PHI,1,1), v3(PHI,-1,1), v3(PHI,1,-1), v3(PHI,-1,-1),
      v3(-PHI,1,1), v3(-PHI,-1,1), v3(-PHI,1,-1), v3(-PHI,-1,-1),
      v3(1,PHI,1), v3(1,PHI,-1), v3(-1,PHI,1), v3(-1,PHI,-1),
      v3(1,-PHI,1), v3(1,-PHI,-1), v3(-1,-PHI,1), v3(-1,-PHI,-1)
    ],
    edges: edgesFromPairs([
      [0,6],[0,7],[0,8],[0,9],[1,10],[1,11],[1,12],[1,13],
      [2,14],[2,15],[2,16],[2,17],[3,18],[3,19],[3,20],[3,21],
      [4,22],[4,23],[4,24],[4,25],[5,26],[5,27],[5,28],[5,29],
      [6,8],[6,14],[6,22],[7,9],[7,15],[7,26],
      [8,18],[8,24],[9,19],[9,28],[10,12],[10,16],[10,23],
      [11,13],[11,17],[11,27],[12,20],[12,25],[13,21],[13,29],
      [14,16],[14,22],[15,17],[15,26],[16,23],[17,27],
      [18,20],[18,24],[19,21],[19,28],[20,25],[21,29],
      [22,24],[23,25],[26,28],[27,29]
    ])
  }
};

let solidCache = {};

function getSolidGeometry(id) {
  return SOLID_DB[id] || SOLID_DB[0];
}

/* Fetch solid geometry from C --serve /solid endpoint */
async function fetchSolid(id) {
  if (solidCache[id]) return solidCache[id];
  try {
    const res = await fetch('/solid?id=' + id);
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const data = await res.json();
    const verts = [];
    for (let i = 0; i < data.verts.length; i += 3)
      verts.push({x: data.verts[i], y: data.verts[i+1], z: data.verts[i+2]});
    const edges = [];
    for (let i = 0; i < data.edges.length; i += 2)
      edges.push({a: data.edges[i], b: data.edges[i+1]});
    const solid = {name: data.name, verts, edges};
    solidCache[id] = solid;
    return solid;
  } catch (e) {
    console.warn('fetchSolid(' + id + ') failed, falling back to SOLID_DB:', e);
    return getSolidGeometry(id);
  }
}

function getSolidId(name) {
  for (const [id, s] of Object.entries(SOLID_DB)) {
    if (s.name === name) return parseInt(id);
  }
  return 0;
}

function fnv1a32(data) {
  let h = 0x811c9dc5 >>> 0;
  for (let i = 0; i < data.length; i++) {
    h ^= data.charCodeAt(i) & 0xff;
    h = Math.imul(h, 0x01000193) >>> 0;
  }
  return h;
}

function modPos(x, m) { return ((x % m) + m) % m; }

function bqf32(x, y) {
  return (60 * x * x) + (16 * x * y) + (4 * y * y);
}

function polybiusGetInterior(x, y) {
  if (x < 0 || x > 3 || y < 0 || y > 3) return null;
  return { row: y + 2, col: x + 2 };
}

function resolveHopfQuquartRoute(chart11, baseQ, fiberQ, fano7, role3) {
  const g = {};
  g.chart11 = modPos(chart11, 11);
  g.baseQ = modPos(baseQ, 4);
  g.fiberQ = modPos(fiberQ, 4);
  g.fano7 = modPos(fano7, 7);
  g.role3 = modPos(role3, 3);

  const x = g.baseQ, y = g.fiberQ;
  g.qxy = bqf32(x, y);
  g.local240 = g.qxy % 240;
  g.slot5040 = g.fano7 * 720 + g.role3 * 240 + g.local240;

  const TWIN_PI = 3.14159265358979323846;
  const theta = ((x + 0.5) / 4.0) * TWIN_PI;
  const phi = ((y + 0.5) / 4.0) * 2.0 * TWIN_PI;
  const halfTheta = theta / 2.0;
  g.qw = Math.cos(halfTheta);
  g.qx = Math.sin(halfTheta) * Math.cos(phi);
  g.qy = Math.sin(halfTheta) * Math.sin(phi);
  g.qz = 0.0;
  g.fiberPhase = phi;

  const w = g.qw, qx = g.qx, qy = g.qy, qz = g.qz;
  g.a = Math.round(2.0 * (qx * qz + w * qy));
  g.b = Math.round(2.0 * (qy * qz - w * qx));
  g.c = Math.round(1.0 - 2.0 * (qx * qx + qy * qy));

  const local16 = g.local240 & 0x0f;
  const px = local16 & 3;
  const py = (local16 >> 2) & 3;
  const cell = polybiusGetInterior(px, py);
  if (cell) {
    g.polybiusRow = cell.row;
    g.polybiusCol = cell.col;
  } else {
    g.polybiusRow = 2;
    g.polybiusCol = 2;
  }

  g.frameType = baseQ & 3;
  return g;
}

/* Total time: 0.037s */
