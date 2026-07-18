# pi_forwarder

Minimal C++ scaffold for a UDP <-> RSSP1 forwarder.

The current executable is a placeholder that builds, starts, prints a startup message, and exits successfully.

By default, the build does not compile the vendored RSSP1 C sources yet. That keeps the scaffold buildable while the real RSSP1 integration is still incomplete.

## Get Standalone ASIO

Use standalone ASIO, not Boost.Asio.

Download a recent standalone ASIO release, for example `asio-1.30.2.zip`, from the official ASIO release page.

After unpacking, make sure one of these paths exists:

```text
third-party/asio-1.36.0/include/asio.hpp
```

The first layout matches your current repo. The other layouts also work.

Quick setup steps on Windows:

1. Download the standalone ASIO zip archive.
2. Create `third-party` or `third_party` if needed.
3. Unzip the archive so that `asio.hpp` ends up under one of the supported paths above.
4. Reconfigure CMake.




## Cheatsheet

All commands are run from the repository root in PowerShell.

### Configure

Generate the build system into `build/`. CMake picks the newest Visual Studio it finds. Re-run this after adding the ASIO headers or changing any CMake option:

```powershell
cmake -S . -B build
```

Same, but also compile the vendored RSSP1 C sources (off by default):

```powershell
cmake -S . -B build -DPI_FORWARDER_ENABLE_VENDOR_RSSP1=ON
```

### Build

Build the default configuration (Debug for Visual Studio generators):

```powershell
cmake --build build
```

Build Release instead — with a multi-config generator such as Visual Studio, the configuration is chosen at build time, not at configure time:

```powershell
cmake --build build --config Release
```

### Run

Visual Studio generators place the executable in a per-configuration subfolder:

```powershell
.\build\Debug\forwarder.exe
.\build\Release\forwarder.exe
```

Single-config generators such as Ninja place it directly in the build folder:

```powershell
.\build\forwarder.exe
```

### Clean

Delete the build folder entirely; the next configure starts from scratch:

```powershell
Remove-Item -Recurse -Force .\out
```