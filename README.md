# Gothic Multiplayer Launcher + SystemPack

This project modernizes the SystemPack for Gothic 2 NotR and also includes optional injection for the Gothic Multiplayer Mod (GMP).
It can also be extended to inject the D3D11 renderer while keeping the original DirectX-7 renderer in place.

## Building
This project was built and tested with VS2026 (v145 Platform Toolset + Windows SDK 10.0.26100.0).  
You will need C++ MFC for x64/x86 (Latest MSVC) and C++/CLI Support (Latest MSVC) with .NET Framework SDK and Targeting Pack v4.8 as well.

### Steps
1. Make sure submodules are pulled (`git submodule update --init`)
2. Next we want to build the ogg library first, then the vorbis library.
   When opening the solutions, upgrade the Platform Toolset to v145 and Windows SDK to 10.0.26100.0.  
   Then set the configuration to `Win32 - Release`.  
   Start with ogg:  
   Open `Vdfs32g\lib\ogg\win32\VS2015\libogg.sln` with Visual Studio and build libogg.  
   Output: `Vdfs32g\lib\ogg\win32\VS2015\Win32\Release\libogg.lib`
3. Next we want to build vorbis.
   We need to make sure that it can find our compiled libogg.lib.  
   Go to `Vdfs32g\lib\vorbis\win32\VS2010` and replace the file `libogg.props` with the one from [Vdfs32g\lib](Vdfs32g\lib\libogg.props)
4. Open `Vdfs32g\lib\vorbis\win32\VS2010\vorbis_static.sln` with Visual Studio and build libvorbis_static and libvorbisfile.  
   Output: `Vdfs32g\lib\vorbis\win32\VS2010\Win32\Release\libvorbis_static.lib`
   and `Vdfs32g\lib\vorbis\win32\VS2010\Win32\Release\libvorbisfile_static.lib`
5. Now we can build the SystemPack and GMP Launcher.  
   Open `Vdfs32g.sln` and build the solution.
