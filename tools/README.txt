// Dependency installers //
Automatically clone dependencies from git, compile and install
Also copy the DLLs to the project's binary folders

Put dependency source folder in libraries/sources
Folder name should coincide with the name specified in Install-[name].js
Once installation is complete, folders in libraries/sources may be deleted.

Run in cmd:
node Install-[name].js [--copy] [--noclone]

--copy: if you have a pre-installed library with installation files in
		libraries/sources/name/install/
									|- bin/
									|- include/
									|- lib/
		only perform the copying steps
		
--noclone: skip the cloning step and only compile and copy

Setup.bat launches scripts for all dependencies (very long process)
Modifiers [--copy] and [--noclone] apply to all .js scripts

Install-Qt.js requires vcpkg.exe