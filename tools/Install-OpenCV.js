import { InstallLibrary } from "./Install.js";

const name = "OpenCV";
const repo = "https://github.com/opencv/opencv.git"
let cmakeOptions = [" -DBUILD_SHARED_LIBS=OFF",
" -DBUILD_TESTS=OFF",
" -DBUILD_PERF_TESTS=OFF",
" -DBUILD_EXAMPLES=OFF",
" -DBUILD_opencv_apps=ON"];

InstallLibrary({name, repo, cmakeSourceSub : "", cmakeOptions});