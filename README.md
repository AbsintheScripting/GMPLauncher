# Gothic Multiplayer Launcher + SystemPack

Gothic games fixes

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
   Go to `Vdfs32g\lib\vorbis\win32\VS2010` and update the file `libogg.props` to:
```xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Label="UserMacros">
    <LIBOGG_VERSION>1.3.6</LIBOGG_VERSION>
  </PropertyGroup>
  <PropertyGroup>
    <_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>$(SolutionDir)..\..\..\ogg\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <RuntimeLibrary Condition="'$(Configuration)'=='Debug'">MultiThreadedDebug</RuntimeLibrary>
      <RuntimeLibrary Condition="'$(Configuration)'=='Release'">MultiThreaded</RuntimeLibrary>
      <AdditionalOptions Condition="'$(Configuration)'=='Debug'">/MTd %(AdditionalOptions)</AdditionalOptions>
      <AdditionalOptions Condition="'$(Configuration)'=='Release'">/MT %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <AdditionalLibraryDirectories>$(SolutionDir)..\..\..\ogg\win32\VS2015\$(PlatformName)\$(ConfigurationName)</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <BuildMacro Include="LIBOGG_VERSION">
      <Value>$(LIBOGG_VERSION)</Value>
    </BuildMacro>
  </ItemGroup>
</Project>
```
4. Open `Vdfs32g\lib\vorbis\win32\VS2010\vorbis_static.sln` with Visual Studio and build libvorbis_static and libvorbisfile.  
   Output: `Vdfs32g\lib\vorbis\win32\VS2010\Win32\Release\libvorbis_static.lib`
   and `Vdfs32g\lib\vorbis\win32\VS2010\Win32\Release\libvorbisfile_static.lib`
5. Now we can build the SystemPack and GMP Launcher.  
   Open `Vdfs32g.sln` and build the solution.
