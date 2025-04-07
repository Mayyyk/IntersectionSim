const express = require("express");
const multer = require("multer");
const cors = require("cors");
const fs = require("fs");
const path = require("path");
const { spawnSync } = require("child_process");

const app = express();
const upload = multer({ dest: "uploads/" });

app.use(cors());
app.use(express.static(path.join(__dirname, "../frontend")));

function runCommand(command, args) {
  const result = spawnSync(command, args, { encoding: "utf-8" });

  if (result.stdout) console.log(result.stdout);
  if (result.stderr) console.error(result.stderr);

  if (result.status !== 0) {
    throw new Error(`Command failed: ${command} ${args.join(" ")}`);
  }
}

app.post("/simulate", upload.single("commands"), (req, res) => {
  try {
    const originalPath = req.file.path;
    const commandsPath = path.join(__dirname, "commands.json");

    fs.renameSync(originalPath, commandsPath);

    // 🔁 Odpal pipeline z logami
    runCommand("python", ["run_simulation.py", "commands.json", "fixed_output.json"]);

    res.download("fixed_output.json", "result.json");
  } catch (err) {
    console.error("❌ Błąd w pipeline:", err.message);
    res.status(500).send("Błąd podczas przetwarzania symulacji.");
  }
});

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`🚀 Serwer działa na http://localhost:${PORT}`);
});
