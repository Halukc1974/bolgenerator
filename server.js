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
    let { majord, length, pitch, headD1, headD2, headD3, headType } = req.body;

    // Validate inputs
    if (!majord || !length || !pitch || !headD1 || !headD2 || !headD3 || headType === undefined) {
        return res.status(400).json({ error: 'All parameters are required' });
    }

    // Convert from mm to meters (user inputs in mm, app expects meters)
    majord = parseFloat(majord) / 1000;
    length = parseFloat(length) / 1000;
    pitch = parseFloat(pitch) / 1000;
    headD1 = parseFloat(headD1) / 1000;
    headD2 = parseFloat(headD2) / 1000;
    headD3 = parseFloat(headD3) / 1000;
    headType = parseInt(headType);

    // Generate a unique filename
    const filename = `bolt_${Date.now()}`;

    // Run the C++ executable (note: order is majord, pitch, length, headD1, headD2, headD3, headType)
    const command = `./scim_bolts ${filename} ${majord} ${pitch} ${length} ${headD1} ${headD2} ${headD3} ${headType}`;

    exec(command, (error, stdout, stderr) => {
        if (error) {
            console.error(`Error: ${error.message}`);
            return res.status(500).json({ error: 'Failed to generate bolt. Check parameters.' });
        }

        const brepPath = path.join(__dirname, 'Tests', `${filename}.brep`);
        const stlPath = path.join(__dirname, 'Tests', `${filename}.stl`);

        // Check if files exist
        if (fs.existsSync(brepPath) && fs.existsSync(stlPath)) {
            // Send JSON response with download URL and filename
            res.json({ 
                success: true,
                filename: filename,
                downloadUrl: `/download/${filename}.brep`,
                previewUrl: `/preview/${filename}.stl`
            });
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
