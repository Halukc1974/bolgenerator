const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const port = process.env.PORT || 3000;

app.use(express.urlencoded({ extended: true }));
app.use(express.json());
app.use(express.static('public'));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.post('/generate', (req, res) => {
    const p = req.body;

    // Debug: Log received parameters
    console.log('Received parameters for generation:', p);

    const filename = `bolt_${Date.now()}`;

    // Map frontend fields to CLI arguments (Matching main.cpp order)
    const args = [
        filename,
        p.headType || 0,
        p.widthAcrossFlats || 0,
        p.headHeight || 0,
        p.washerFaceDiameter || 0,
        p.washerFaceThickness || 0,
        p.underheadFilletRadius || 0,
        p.socketSize || 0,
        p.socketDepth || 0,
        p.nominalDiameter || 0,
        p.totalLength || 0,
        p.gripLength || 0,
        p.bodyTolerance || 0,
        p.majorDiameter || 0,
        p.threadPitch || 0,
        p.minorDiameter || 0,
        p.generateNut ? 1 : 0,
        p.nutAcrossFlats || 0,
        p.nutHeight || 0,
        p.nutWasherFace || 0,
        p.nutTolerance || 0
    ];

    const command = `./scim_bolts ${args.join(' ')}`;
    console.log('Executing:', command);

    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Generation error: ${error.message}`);
            return res.status(500).json({
                success: false,
                error: "Geometry generation failed. Check parameters (especially pitch vs diameter)."
            });
        }

        const result = {
            success: true,
            filename: filename,
            boltBrep: `/download/${filename}.brep`,
            boltStl: `/preview/${filename}.stl`
        };

        if (p.generateNut) {
            result.nutBrep = `/download/${filename}_nut.brep`;
            result.nutStl = `/preview/${filename}_nut.stl`;
        }

        res.json(result);
    });
});

app.get('/preview/:filename', (req, res) => {
    const file = path.join(__dirname, 'Tests', req.params.filename);
    if (fs.existsSync(file)) res.sendFile(file);
    else res.status(404).json({ error: 'File not found' });
});

app.get('/download/:filename', (req, res) => {
    const file = path.join(__dirname, 'Tests', req.params.filename);
    if (fs.existsSync(file)) res.download(file, req.params.filename);
    else res.status(404).json({ error: 'File not found' });
});

app.listen(port, () => {
    console.log(`BoltGenerator (v2) listening on port ${port}`);
});
