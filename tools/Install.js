// Generic script to clone and install library
// Run: node Install-[libname] [--copy]

import { execSync } from "child_process";
import { existsSync, mkdirSync, copyFileSync, readdirSync, lstatSync } from "fs";
import { dirname, join, resolve } from "path";
import { fileURLToPath } from "url";

// Fix __dirname in ES module
const __filename = dirname(fileURLToPath(import.meta.url));

const ProjectRoot = resolve(__filename, "..");
const LibSrc = join(ProjectRoot, "libraries", "sources");
const LibInstall = join(ProjectRoot, "libraries", "install");
const BuildDir = join(ProjectRoot, "build");

const BuildTypes = ["Debug", "Release"];

// Clone, compile and install the library
export function InstallLibrary({ name, repo, cmakeSourceSub = "", cmakeOptions = [], options = process.argv })
{
    const srcDir = join(LibSrc, name);
    const cmakeSourceDir = join(srcDir, cmakeSourceSub)
    const installDirLoc = join(LibSrc, "install");
    const installDir = join(LibInstall, name);

    const run = (cmd, cwd = ProjectRoot) => {
        try{
            console.log(`>> ${cmd}`);
            execSync(cmd, { stdio: "inherit", cwd });
        }
        catch(err){
            console.error(`\n[ERROR] Failed while running: ${cmd}`);
            process.exit(1);
        }
    };

    const copyRecursive = (src, dest) => {
        try{
            if (!existsSync(src)) return;
            if (!existsSync(dest)) mkdirSync(dest, { recursive: true });
            for (const item of readdirSync(src)){
                const s = join(src, item);
                const d = join(dest, item);
                if (lstatSync(s).isDirectory()) copyRecursive(s, d);
                else copyFileSync(s, d);
            }
        }
        catch (err) {
            console.error(`\n[ERROR] Failed while copying from ${src} to ${dest}`);
            console.error(err.message);
            process.exit(1);
        }
    };

    const copyOnly = options.includes("--copy"); //If the lib is already installed
    const noclone = options.includes("--noclone"); //If the lib src is already cloned

    console.log(`\n=== Installing ${name} ===`);

    if(!copyOnly){
        // Step 1: clone repo from git
        if(!noclone){
            if(!existsSync(srcDir)){
                mkdirSync(LibSrc, {recursive: true});
                run(`git clone ${repo} ${srcDir}`);
            }
            else{
                console.log(`${name}: source directory already exists, skipping clone.`)
            }
        }
        // Step 2: build debug and release
        for (const type of BuildTypes){
            const buildDir = join(srcDir, "build", type);
            mkdirSync(buildDir, {recursive: true});

            const cmakeCmd = [
                "cmake",
                `-DCMAKE_BUILD_TYPE=${type}`,
                `-DCMAKE_INSTALL_PREFIX=${installDir}`,
                `-DCMAKE_DEBUG_POSTFIX=-d`,
                `-DCMAKE_INSTALL_BINDIR=bin/${type}`,
                `-DCMAKE_INSTALL_LIBDIR=lib/${type}`,
                `-DCMAKE_INSTALL_INCLUDEDIR=include`,
                ...cmakeOptions,
                cmakeSourceDir,
            ].join(" ");

            run(cmakeCmd, buildDir);
            run(`cmake --build . --config ${type} --target install`, buildDir);
        }
    }

    // Step 3: copy installed lib to project libraries folder
    console.log(`Copying ${name} install output to libraries/install/${name}...`);
    copyRecursive(installDirLoc, installDir);

    // Step 4: copy DLLs to the build folders
    console.log(`Copying ${name} DLLs to project build folders...`);

    for (const type of BuildTypes) {
        const binDir = join(installDir, "bin", type);
        if (!existsSync(binDir)) continue;

        const dlls = readdirSync(binDir).filter(f => f.endsWith(".dll"));
        const dest = join(BuildDir, type);
        mkdirSync(dest, { recursive: true });

        for (const dll of dlls) {
            copyFileSync(join(binDir, dll), join(dest, dll));
        }

        // Also copy Release DLLs to variants
        if (type === "Release") {
            for (const cfg of ["RelWithDebInfo", "MinSizeRel"]) {
                const variantDest = join(BuildDir, cfg);
                mkdirSync(variantDest, { recursive: true });
                for (const dll of dlls)
                    copyFileSync(join(binDir, dll), join(variantDest, dll));
            }
        }
    }

  console.log(`[${name}] Installation complete.`);
}

export { ProjectRoot, BuildDir };