const express = require('express');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const port = process.env.PORT || 3000;

app.use(express.urlencoded({ extended: true }));
app.use(express.json());
app.use(express.static('public'));

const TESTS_DIR = path.join(__dirname, 'Tests');

// Whitelist: only files produced by scim_bolts are downloadable.
const SAFE_FILENAME = /^bolt_\d+(_nut)?\.(brep|stl|step|stp)$/;

function resolveSafeTestsPath(filename) {
    if (!SAFE_FILENAME.test(filename)) return null;
    const base = path.basename(filename);
    const resolved = path.resolve(TESTS_DIR, base);
    if (!resolved.startsWith(TESTS_DIR + path.sep)) return null;
    return resolved;
}

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.post('/generate', (req, res) => {
    const p = req.body;

    console.log('Received parameters for generation:', p);

    const filename = `bolt_${Date.now()}`;

    const L = parseFloat(p.totalLength) || 10;
    const grip = parseFloat(p.gripLength) || 0;
    const pitch = parseFloat(p.threadPitch) || 1.25;
    const d = parseFloat(p.nominalDiameter) || 8;

    const clampedGrip = Math.min(grip, L - (2 * pitch));
    // Use the pitch parsed from threadPitch (above); previous code read a
    // non-existent `p.pitch` and always clamped to d*0.2.
    // Lower floor is 0.2 so M3 (0.5) doesn't get clamped up to 1.0.
    const clampedPitch = Math.max(0.2, Math.min(pitch, d * 0.3));

    // Clamp fillet radii to safe maximum (10% of relevant dimension)
    const maxBoltFillet = d * 0.1;
    const maxNutFillet = (p.nutAcrossFlats || d * 1.5) * 0.1;
    const clampedBoltFillet = Math.max(0, Math.min(p.edgeFilletRadius || 0.2, maxBoltFillet));
    const clampedNutFillet = Math.max(0, Math.min(p.nutEdgeFilletRadius || 0.2, maxNutFillet));

    const args = [
        filename,
        String(p.headType || 0),
        String(p.widthAcrossFlats || 0),
        String(p.headHeight || 0),
        String(p.washerFaceDiameter || 0),
        String(p.washerFaceThickness || 0),
        String(p.underheadFilletRadius || 0),
        String(p.socketSize || 0),
        String(p.socketDepth || 0),
        String(d),
        String(L),
        String(clampedGrip),
        String(p.bodyTolerance || 0),
        String(p.majorDiameter || d),
        String(clampedPitch),
        String(p.minorDiameter || 0),
        String(p.generateNut ? 1 : 0),
        String(p.nutAcrossFlats || 0),
        String(p.nutHeight || 0),
        String(p.nutWasherFace || 0),
        String(p.nutTolerance || 0.15),
        String(clampedBoltFillet),
        String(clampedNutFillet)
    ];

    console.log('Executing: ./scim_bolts', args.join(' '));

    // execFile prevents shell injection (safer than exec with string concatenation)
    execFile('./scim_bolts', args, { cwd: __dirname }, (error, stdout, stderr) => {
        if (error) {
            console.error(`Generation error: ${error.message}`);
            console.error(`stderr: ${stderr}`);
            return res.status(500).json({
                success: false,
                error: "Geometry generation failed. Check parameters (especially pitch vs diameter)."
            });
        }

        // Parse optional mass/volume stats from stdout (emitted by export.cpp)
        const stats = parseGeometryStats(stdout);

        const result = {
            success: true,
            filename: filename,
            boltBrep: `/download/${filename}.brep`,
            boltStl: `/preview/${filename}.stl`,
            boltStep: `/download/${filename}.step`,
            stats: stats.bolt
        };

        if (p.generateNut) {
            result.nutBrep = `/download/${filename}_nut.brep`;
            result.nutStl = `/preview/${filename}_nut.stl`;
            result.nutStep = `/download/${filename}_nut.step`;
            result.nutStats = stats.nut;
        }

        res.json(result);
    });
});

function parseGeometryStats(stdout) {
    // Parses lines like:
    //   "Bolt Volume: 1234.56 mm^3"
    //   "Nut Volume: 567.89 mm^3"
    const stats = { bolt: null, nut: null };
    const boltMatch = stdout.match(/Bolt Volume:\s*([\d.]+)\s*mm\^3/);
    const nutMatch = stdout.match(/Nut Volume:\s*([\d.]+)\s*mm\^3/);
    if (boltMatch) stats.bolt = { volume_mm3: parseFloat(boltMatch[1]) };
    if (nutMatch) stats.nut = { volume_mm3: parseFloat(nutMatch[1]) };
    return stats;
}

app.get('/preview/:filename', (req, res) => {
    const safe = resolveSafeTestsPath(req.params.filename);
    if (!safe) return res.status(400).json({ error: 'Invalid filename' });
    if (fs.existsSync(safe)) res.sendFile(safe);
    else res.status(404).json({ error: 'File not found' });
});

app.get('/download/:filename', (req, res) => {
    const safe = resolveSafeTestsPath(req.params.filename);
    if (!safe) return res.status(400).json({ error: 'Invalid filename' });
    if (fs.existsSync(safe)) res.download(safe, path.basename(safe));
    else res.status(404).json({ error: 'File not found' });
});

app.listen(port, () => {
    console.log(`BoltGenerator (v2) listening on port ${port}`);
});
