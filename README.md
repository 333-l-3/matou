# matou

C++17 tower-defense project built with SFML (PVZ style).

## Version
- Current release: `v0.3`
- Release date: `2026-03-05`
- Repository: `https://github.com/333-l-3/matou.git`

## v0.3 Highlights
- Potato mine damage area changed to cross-shaped 5 tiles (self + up/down/left/right).
- Build and runtime documentation expanded.
- Cross-machine setup notes added (SFML runtime DLL + resource folder requirements).

## Requirements
- OS: Windows 10/11 (x64)
- Compiler: MinGW g++ (recommended 13.x)
- Language standard: `C++17`
- SFML version: `2.6.2`
- Extra system library: `GDI+` (`-lgdiplus`)

## Resource Pack (`res`)
- The game requires a `res/` folder at project root (images, audio, CSV configs).
- Resource download link (replace after your review): `<RES_DOWNLOAD_URL>`
- Expected result after extract: `matou/res/...`

## Build
1. Prepare folders
- Put SFML SDK at project root: `SFML2.6.2/`
- Put resources at project root: `res/`

2. Build in VS Code
- Use the build task in `.vscode/tasks.json` to generate `bin/debug/main.exe`.

3. Manual build command (equivalent)
```powershell
C:\work\os\mingw64\bin\g++.exe `
  -fdiagnostics-color=always -g -m64 `
  -D_GLIBCXX_USE_CXX11_ABI=0 -U_GLIBCXX_USE_CXX11_ABI `
  src\main.cpp `
  src\scene\LevelSelectScene.cpp src\scene\LoadoutScene.cpp src\scene\PlantListScene.cpp src\scene\Level1Scene.cpp `
  src\battle\StatsDatabase.cpp src\battle\BattleSimulator.cpp src\battle\PlantAttackSystem.cpp `
  -I src\tool -I src\window -I src\scene -I src\battle -I SFML2.6.2\include `
  -L SFML2.6.2\lib -lsfml-graphics-d -lsfml-window-d -lsfml-system-d -lgdiplus `
  -std=c++17 -static-libstdc++ -Wl,--disable-auto-import `
  -o bin\debug\main.exe
```

## Runtime Notes
- Runtime does not use `.o` files; it only runs `main.exe`.
- SFML runtime DLLs must be discoverable (same folder as `main.exe`, or added to `PATH`).

## Project Structure
```text
matou/
|-- .vscode/                 # VS Code task/debug settings
|-- bin/
|   `-- debug/               # build outputs (main.exe / *.o)
|-- src/
|   |-- battle/              # simulator and attack logic
|   |-- scene/               # scenes and level flow
|   |-- tool/                # file/resource utilities
|   |-- window/              # window and main loop wrapper
|   |-- entry/               # legacy/example entities
|   `-- main.cpp
|-- res/                     # game assets (may be gitignored)
|-- SFML2.6.2/               # local SFML SDK (may be gitignored)
|-- sfml-graphics-d-2.dll    # runtime DLL example
|-- sfml-window-d-2.dll      # runtime DLL example
|-- sfml-system-d-2.dll      # runtime DLL example
|-- README.md
`-- .gitignore
```

## Minimal Sharing Checklist
- Always commit: `src/`, `.vscode/`, `README.md`, `.gitignore`
- Must exist for running (even if not committed): `res/`, SFML runtime DLLs
- If `SFML2.6.2/` is ignored, collaborators must provide that SDK locally.
