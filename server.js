const express = require("express");
const fileUpload = require("express-fileupload");
const cors = require("cors");
const path = require("path");
const fs = require("fs");

const app = express();

// ================= VERSI =================
const versionFile = path.join(__dirname, "version.txt");

// ✅ Baca dari file, tidak hilang saat restart
let currentVersion = fs.existsSync(versionFile)
  ? fs.readFileSync(versionFile, "utf8").trim()
  : "1.0";

console.log(`📋 Versi saat ini: ${currentVersion}`);

// ================= MIDDLEWARE =================
app.use(cors({
  origin: "*",
  methods: ["GET", "POST", "OPTIONS"],
  allowedHeaders: ["Content-Type", "ngrok-skip-browser-warning", "User-Agent", "x-firmware-version"],
  preflightContinue: false,
  optionsSuccessStatus: 204
}));

// ================= VERSION =================
app.get("/version", (req, res) => {
  console.log(`📋 Versi diminta: ${currentVersion}`);
  res.send(currentVersion);
});

// ================= DOWNLOAD =================
app.get("/firmware.bin", (req, res) => {
  const filePath = path.join(__dirname, "firmware.bin");
  if (!fs.existsSync(filePath)) {
    return res.status(404).send("❌ File belum ada");
  }
  console.log("📥 Firmware diminta oleh client");
  res.setHeader("Content-Type", "application/octet-stream");
  res.sendFile(filePath);
});

// ================= STATIC =================
app.use(express.static(path.join(__dirname, "public")));

// ================= FILE UPLOAD =================
app.use(fileUpload({
  limits: { fileSize: 10 * 1024 * 1024 },
}));

// ================= LOG =================
app.use((req, res, next) => {
  console.log(`[${req.method}] ${req.url}`);
  next();
});

// ================= UPLOAD =================
app.post("/upload", (req, res) => {
  try {
    if (!req.files || !req.files.file) {
      return res.status(400).send("❌ Tidak ada file");
    }

    const file = req.files.file;

    if (!file.name.endsWith(".bin")) {
      return res.status(400).send("❌ Harus file .bin");
    }

    const newVersion = req.headers["x-firmware-version"];
    if (newVersion) {
      currentVersion = newVersion;
      fs.writeFileSync(versionFile, currentVersion); // ✅ simpan ke file
      console.log(`🔖 Versi diupdate ke: ${currentVersion}`);
    }

    const savePath = path.join(__dirname, "firmware.bin");
    file.mv(savePath, (err) => {
      if (err) {
        console.error("Upload error:", err);
        return res.status(500).send("❌ Upload gagal");
      }
      console.log("✅ Firmware berhasil diupload");
      res.send("✅ Upload sukses!");
    });

  } catch (err) {
    console.error(err);
    res.status(500).send("❌ Server error");
  }
});

// ================= START =================
const PORT = 5000;
app.listen(PORT, () => {
  console.log(`🚀 Server jalan di http://localhost:${PORT}`);
});

process.stdin.resume();