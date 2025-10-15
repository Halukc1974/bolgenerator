const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const port = 3000;

// Middleware to parse form data
app.use(express.urlencoded({ extended: true }));
app.use(express.json());

// Serve static files (HTML, CSS, JS)
app.use(express.static('public'));

// Route for the main page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Route to generate bolt
app.post('/generate', (req, res) => {
    let { majord, length, pitch, headD1, headD2, headD3, headD4, headType, 
          generateNut, nutHeight, nutAcrossFlats, tolerance } = req.body;

    // Debug: Log received parameters
    console.log('Received parameters:', { majord, length, pitch, headD1, headD2, headD3, headD4, headType, generateNut, nutHeight, nutAcrossFlats, tolerance });

    // Validate inputs
    if (!majord || !length || !pitch || !headD1 || !headD2 || headType === undefined) {
        return res.status(400).json({ error: 'Major diameter, length, pitch, headD1, headD2, and headType are required' });
    }

    // Keep values in millimeters (no conversion)
    majord = parseFloat(majord);
    length = parseFloat(length);
    pitch = parseFloat(pitch);
    headD1 = parseFloat(headD1);
    headD2 = parseFloat(headD2);
    headD3 = parseFloat(headD3) || 0;
    headD4 = parseFloat(headD4) || 0;
    headType = parseInt(headType);
    
    // Nut parameters
    generateNut = generateNut === 'true' || generateNut === true || generateNut === 1 || generateNut === '1';
    nutHeight = parseFloat(nutHeight) || 5;
    nutAcrossFlats = parseFloat(nutAcrossFlats) || 10;
    tolerance = parseFloat(tolerance) || 0.1;

    console.log('Parsed parameters:', { majord, length, pitch, headD1, headD2, headD3, headD4, headType, generateNut, nutHeight, nutAcrossFlats, tolerance });

    // Validate headD3 and headD4 for socket head
    if (headType === 1) {
        if (!headD3 || headD3 <= 0) {
            return res.status(400).json({ error: 'Socket head requires headD3 (socket size)' });
        }
        if (!headD4 || headD4 <= 0) {
            return res.status(400).json({ error: 'Socket head requires headD4 (socket depth)' });
        }
    }

    // Generate a unique filename
    const filename = `bolt_${Date.now()}`;

    // Run the C++ executable with nut parameters
    const nutFlag = generateNut ? 1 : 0;
    const command = `./scim_bolts ${filename} ${majord} ${pitch} ${length} ${headD1} ${headD2} ${headD3} ${headD4} ${headType} ${nutHeight} ${nutAcrossFlats} ${tolerance} ${nutFlag}`;

    console.log('Executing command:', command);

    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error: ${error.message}`);
            console.error(`stderr: ${stderr}`);
            return res.status(500).json({ error: 'Failed to generate bolt. Check parameters.' });
        }

        const brepPath = path.join(__dirname, 'Tests', `${filename}.brep`);
        const stlPath = path.join(__dirname, 'Tests', `${filename}.stl`);
        const nutBrepPath = path.join(__dirname, 'Tests', `${filename}_nut.brep`);
        const nutStlPath = path.join(__dirname, 'Tests', `${filename}_nut.stl`);

        // Check if files exist
        if (fs.existsSync(brepPath) && fs.existsSync(stlPath)) {
            const response = { 
                success: true,
                filename: filename,
                downloadUrl: `/download/${filename}.brep`,
                previewUrl: `/preview/${filename}.stl`
            };
            
            // Add nut URLs if nut was generated
            if (generateNut && fs.existsSync(nutBrepPath) && fs.existsSync(nutStlPath)) {
                response.nutDownloadUrl = `/download/${filename}_nut.brep`;
                response.nutPreviewUrl = `/preview/${filename}_nut.stl`;
                response.nutGenerated = true;
            }
            
            res.json(response);
        } else {
            res.status(500).json({ error: 'Generated files not found' });
        }
    });
});

// Route to get STL file for preview
app.get('/preview/:filename', (req, res) => {
    const filename = req.params.filename;
    const stlPath = path.join(__dirname, 'Tests', filename);
    
    if (fs.existsSync(stlPath)) {
        res.sendFile(stlPath);
    } else {
        res.status(404).json({ error: 'STL file not found' });
    }
});

// Route to download BREP file
app.get('/download/:filename', (req, res) => {
    const filename = req.params.filename;
    const brepPath = path.join(__dirname, 'Tests', filename);
    
    if (fs.existsSync(brepPath)) {
        res.download(brepPath, filename);
    } else {
        res.status(404).json({ error: 'BREP file not found' });
    }
});

app.listen(port, () => {
    console.log(`BoltGenerator web app listening at http://localhost:${port}`);
});
