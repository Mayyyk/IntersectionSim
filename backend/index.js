const express = require("express");
const multer = require("multer");
const cors = require("cors");
const fs = require("fs");
const path = require("path");
const { spawnSync } = require("child_process");



const app = express();
const upload = multer({ dest: "uploads/" });

app.use(cors());
app.use(express.static(path.join(__dirname)));

function runCommand(command, args) {
  const result = spawnSync(command, args, {
    encoding: "utf-8",
    shell: true // ważne przy Windows
  });

  console.log(`\n> ${command} ${args.join(" ")}`);

  if (result.stdout) {
    console.log("=== STDOUT ===");
    console.log(result.stdout);
  }

  if (result.stderr) {
    console.log("=== STDERR ===");
    console.error(result.stderr);
  }

  if (result.status !== 0) {
    throw new Error(`Command failed: ${command} ${args.join(" ")}`);
  }
  console.log("Exit code:", result.status);

}




app.post("/simulate", upload.single("commands"), (req, res) => {
  try {
    const tempPath = req.file.path;
    const commandsPath = path.join(__dirname, "commands.json");

    // Najpierw kopiuj plik, dopiero potem usuń tymczasowy
    fs.copyFileSync(tempPath, commandsPath);
    fs.unlinkSync(tempPath); // usuń plik z uploads

    // 🔁 Odpal pipeline
    runCommand("python", ["run_simulation.py", "commands.json", "fixed_output.json"]);

    res.download("fixed_output.json", "result.json");

  } catch (err) {
    console.error("❌ Błąd w pipeline:", err.message);
    console.error(err.stack);
    res.status(500).send("Błąd podczas przetwarzania symulacji.");
  }
});


app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "index.html"));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`🚀 Serwer działa na http://localhost:${PORT}`);
});
