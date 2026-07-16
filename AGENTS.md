# Vdfs32g Project Guide for Agents

## What is Vdfs32g?

Vdfs32g (aka **SystemPack**) is a drop-in replacement DLL for the games *Gothic 1* (2001) and *Gothic 2* (2003) by Piranha Bytes. It replaces the original `vdfs32g.dll` in the game's `System` folder and acts as a mod loader / compatibility layer that:

1. **Patches game memory** at runtime using CRC-matched (Cyclic Redundancy Check) `.patch` files (see KillerFix system)
2. **Hooks Windows API imports** (LoadBitmapA, DeleteFileA, DirectDrawEnumerateExA, etc.) via IAT patching
3. **Provides a virtual file system** that loads `.vdf` and `.mod` archive files from `Data\`, overlaying them on the physical filesystem
4. **Transcodes OGG Vorbis audio** into WAV format on-the-fly for the game's audio system
5. **Fixes various game bugs** (splash screen replacement, save backup, INI settings, DPI awareness, Steam overlay crashes)

The DLL is loaded by the game without validation, giving it full control over the game process memory space.

## Project Structure

```
Vdfs32g/
  Components/
    Fixes/              # Game bug fixes via IAT patching
      KillerFix.cpp/h   # Memory patching engine (loads .patch files by CRC)
      SplashFix.cpp/h   # Custom splash screen replacement
      SteamOverlayFix.cpp/h  # Steam overlay crash fix (ddraw.dll hook)
      SaveBakFix.cpp/h  # Save game backup via DeleteFileA hook
      IniFix.cpp/h      # Gothic.ini settings enforcement
      D3DFix.cpp/h      # DirectX fixes
      FsHook.cpp/h      # Filesystem hooking
      BinkFix.cpp/h     # Bink video fix
      MssFix.cpp/h      # MILES audio fix
      SendMsgFix.cpp/h  # Send message fix
      GUXFix.cpp/h      # GUX (Gothic Utility eXtension) fix
    Common/             # Shared utilities
      Object.h          # Base ref-counted object + AutoPtr/TunablePtr smart pointers
      TypeDefs.h        # Cross-platform type aliases + cast macros + TCHAR mappings
      Utility.h/cpp     # CRC (8/16/32), encryption, string/mem search, math macros
      UtilityEx.h       # Platform API wrappers (GetComputerName, GetTempPath, etc.)
      ComPtr.h          # COM smart pointer wrapper
      Containers/       # Custom containers (Array, HashTable, String, TaggedArray)
    Vdfs/               # Virtual file system
      Vdfs.cpp/h        # Main VFS class (mounts .vdf/.mod, manages flows + filters)
      VdfsIndex.h       # File index (name -> size/offset/flow mapping)
      Flows/
        IFS.h           # Abstract file stream interface (buffered reads, ref counting)
        StdFlow.cpp/h   # Physical filesystem stream (HANDLE-backed)
        VdfFlow.cpp/h   # VDF/MOD archive stream (parses PSVDSC_V2.00 format)
      Filters/
        OggFilter.cpp/h # OGG Vorbis -> WAV transcoder filter
    Bink/               # Bink video codec integration
    PatchUtils.h        # Low-level memory patching utilities
    IniUtils.h/cpp      # INI file reading/writing
    IniUtilsEx.h/cpp    # Extended INI utilities
  lib/                  # Git submodules: ogg/ and vorbis/ (SEE BELOW)
  PreCompiled.h         # Precompiled header
  resource.h            # Resource IDs
  Vdfs32.cpp            # DLL entry point (DllMain) + exported VDFS API functions
```

## Key Concepts

- **IFS (Indexed File System)**: Abstract stream interface. `StdFlow` = physical files, `VdfFlow` = VDF archives. Streams are pooled for concurrency.
- **Filters**: Transform IFS streams. `OggFilter` intercepts OGG files and presents them as WAV streams to the game.
- **VdfsIndex**: Hash-table-based file index mapping names to metadata (size, offset, owning flow).
- **KillerFix**: The patching engine. Reads `.patch` INI files (matched by game EXE CRC), allocates memory blocks, and applies pointer/int/float/hex patches with optional origin verification.
- **IAT Patching**: All hooking is done by patching the Import Address Table of the game process to redirect API calls to custom functions.

## Navigation Tips

- Start with `Vdfs32.cpp` to see exported API functions and DllMain.
- `Components/Fixes.h` lists all fix installation functions.
- `Components/Fixes.cpp` is the orchestrator that calls each `InstallXxxFix()`.
- Look at already-commented files (`IniFix.cpp`, `SaveBakFix.cpp`) for the established comment style.
- Classes get 1-3 line docs. Functions/structs get 1-2 line docs.

## IMPORTANT: lib/ Folder

```
Vdfs32g/
  lib/
    ogg/      # Ogg bitstream library (git submodule)
    vorbis/   # Vorbis codec library (git submodule)
```

**The `lib/` folder contains git submodules for the Ogg and Vorbis libraries. DO NOT modify, edit, or change anything inside `lib/ogg/` or `lib/vorbis/`. These are third-party dependencies that must remain untouched. Any changes to these libraries should be done at the submodule level, not within this project.**
