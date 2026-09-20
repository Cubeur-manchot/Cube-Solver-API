Pre-requisites for local development.

# g++ and gdb
- Download MSYS2 (https://msys2.org)
- Open MSYS2 terminal, run `pacman -S mingw-w64-x86_64-gcc`
- Open MSYS2 terminal, run `pacman -S mingw-w64-x86_64-gdb`
- Update PATH
	- Type Windows+R, enter `sysdm.cpl`
	- Advanced parameters, environment variables
	- Find "Path", update, new, enter `C:\msys64\mingw64\bin`
	- Validate everything to save
- Test Path
	- Open PowerShell terminal
	- Run `g++ --version` (expected : a version and no error)
	- Run `gdb --version` (expected result : a version and no error)

# Make
- Open MSYS2 terminal, run `pacman -S mingw-w64-x86_64-make`
- Test Path (same PATH entry as g++ and gdb, no additional step needed)
	- Open a new PowerShell terminal
	- Run `mingw32-make --version` (expected : a version and no error)
