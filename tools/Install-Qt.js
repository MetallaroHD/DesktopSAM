// Uses vckpg installer instead of cmake compilation

import { execSync } from "child_process";
import { existsSync, mkdirSync, readdirSync, copyFileSync, rmSync, lstatSync } from "fs";
import { join, resolve } from "path";
import { dirname } from "path";
import { fileURLToPath } from "url";

// === Parse argv for vcpkg path ===
const argv = process.argv.slice(2);

let VCPKGPath = null;

// 1) CLI argument
if (argv.length >= 1 && !argv[0].startsWith("--")) {
    VCPKGPath = resolve(argv[0]);
}
// 2) Environment variable fallback (ONLY new feature)
else if (process.env.VCPKG_ROOT) {
    VCPKGPath = resolve(process.env.VCPKG_ROOT);
}

if (!VCPKGPath) {
    console.error("Install-Qt.js requires vcpkg.exe.\n");
    console.error("Usage: node Install-Qt.js <path-to-vcpkg.exe>");
    console.error("Or set the VCPKG_ROOT environment variable.");
    process.exit(1);
}

if (!existsSync(VCPKGPath)) {
    console.error(`vcpkg executable not found at: ${VCPKGPath}`);
    process.exit(1);
}

const __dirname = dirname(fileURLToPath(import.meta.url));

const ProjectRoot = resolve(__dirname, "..");
const LibName = "Qt";
const LibInstall = join(ProjectRoot, "libraries", "install", LibName);
const BuildRoot = join(ProjectRoot, "build");

const run = cmd => {
    console.log(">>", cmd);
    try {
        execSync(cmd, { stdio: "inherit" });
    } catch (err) {
        console.error(`[${LibName}] ERROR: Command failed: ${cmd}`);
        process.exit(1);
    }
};

// Installation
if (!existsSync(LibInstall)) {
    console.log(`[${LibName}] Installing via vcpkg...`);

    run(`"${VCPKGPath}" install qtbase:x64-windows`);

    const vcpkgDir = join("vcpkg", "installed", "x64-windows");

    if (existsSync(LibInstall)) rmSync(LibInstall, { recursive: true, force: true });
    mkdirSync(LibInstall, { recursive: true });

    run(`xcopy /E /I /Y "${vcpkgDir}" "${LibInstall}"`);
}

// Ensure canonical structure: bin, lib, include
const normalizeDirs = ["bin", "lib", "include"];
for (const dir of normalizeDirs) {
    const target = join(LibInstall, dir);
    if (!existsSync(target)) mkdirSync(target, { recursive: true });
}

// Copy DLLs to project build folders
const BinRel = join(LibInstall, "bin");
const BinDeb = join(LibInstall, "debug", "bin");

// Plugin dirs
const PluginsRel = join(LibInstall, "Qt6", "plugins");
const PluginsDeb = join(LibInstall, "debug", "Qt6", "plugins");

// Mapping build configs → source paths
const BuildDirs = {
    Release: { bin: BinRel, plugins: PluginsRel },
    RelWithDebInfo: { bin: BinRel, plugins: PluginsRel },
    MinSizeRel: { bin: BinRel, plugins: PluginsRel },
    Debug: { bin: BinDeb, plugins: PluginsDeb }
};

// Utility: copy folder recursively, applying same Debug/Release filter
function copyFolderFiltered(src, dest, config) {
    if (!existsSync(src)) return;
    mkdirSync(dest, { recursive: true });

    for (const f of readdirSync(src)) {
        const s = join(src, f);
        const d = join(dest, f);

        if (lstatSync(s).isDirectory()) {
            copyFolderFiltered(s, d, config);
        } else {
            const lower = f.toLowerCase();
            const isDebugFile = lower.endsWith("d.dll");

            // same rule as before
            if (config === "Debug" && !isDebugFile && lower.endsWith(".dll")) continue;
            if (config !== "Debug" && isDebugFile) continue;

            copyFileSync(s, d);
        }
    }
}

// Main loop per configuration
for (const [config, { bin, plugins }] of Object.entries(BuildDirs)) {
    const destRoot = join(BuildRoot, config);
    mkdirSync(destRoot, { recursive: true });

    // Copy DLLs
    if (existsSync(bin)) {
        for (const f of readdirSync(bin)) {
            const lower = f.toLowerCase();
            copyFileSync(join(bin, f), join(destRoot, f));
        }
    }

    // Copy the entire "platforms" plugin folder
    const platformSrc = join(plugins, "platforms");
    const platformDest = join(destRoot, "platforms");
    copyFolderFiltered(platformSrc, platformDest, config);

    // (Optional) also copy image format plugins, styles, etc.
    const imgSrc = join(plugins, "imageformats");
    const imgDest = join(destRoot, "imageformats");
    copyFolderFiltered(imgSrc, imgDest, config);
}

console.log(`[${LibName}] Installed and DLLs copied successfully.`);
