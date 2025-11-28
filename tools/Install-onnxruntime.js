import { InstallLibrary, ProjectRoot, BuildDir } from "./Install.js";

const name = "onnxruntime";
const repo = "https://github.com/microsoft/onnxruntime.git"
const cmakeSourceSub = "cmake";
let cmakeOptions = [" -DBUILD_SHARED_LIBS=OFF",
` -DABSL_FIND_GOOGLETEST=OFF`,
` -Donnxruntime_BUILD_UNIT_TESTS=OFF`,
` -Donnxruntime_BUILD_SHARED_LIB=ON`,
` -Donnxruntime_USE_DML=ON`,
` -Dprotobuf_BUILD_SHARED_LIBS=ON`,
` -DCMAKE_CXX_FLAGS="/DWIN32 /D_WINDOWS /EHsc /MP"`,
` -DCMAKE_C_FLAGS="/DWIN32 /D_WINDOWS /MP"`,
` -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"`];

InstallLibrary({name, repo, cmakeSourceSub, cmakeOptions});

//Post install operations
import { join } from "path";
import { copyFileSync } from "fs";

const protoDirDeb = join(ProjectRoot, "/libraries/sources/onnxruntime/build/Debug/_deps/protobuf-build/Debug");
const protoDirRel = join(ProjectRoot, "/libraries/sources/onnxruntime/build/Release/_deps/protobuf-build/Release");

copyFileSync(join(protoDirDeb, "/libprotobuf-lited.dll"), join(BuildDir, "/Debug/libprotobuf-lited.dll"));
copyFileSync(join(protoDirRel, "/libprotobuf-lite.dll"), join(BuildDir, "/RelWithDebInfo/libprotobuf-lite.dll"));
copyFileSync(join(protoDirRel, "/libprotobuf-lite.dll"), join(BuildDir, "/Release/libprotobuf-lite.dll"));