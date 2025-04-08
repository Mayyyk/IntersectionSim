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
  const result = spawnSync(command, args, { encoding: "utf-8" });

  console.log(`\n> ${command} ${args.join(" ")}`);
  if (result.stdout) console.log("STDOUT:", result.stdout);
  if (result.stderr) console.error("STDERR:", result.stderr);

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
    runCommand("python3", ["run_simulation.py", "commands.json", "fixed_output.json"]);

    res.download("fixed_output.json", "result.json");
  } catch (err) {
    console.error("❌ Błąd w pipeline:", err.message);
    console.error(err.stack); // <-- dodaj to
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
