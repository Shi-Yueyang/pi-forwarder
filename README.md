# pi_forwarder

A C++ gateway that receives plain UDP packets from local applications, forwards them over the RSSP1 safety protocol, and extracts received RSSP1 payloads back to UDP.

## Problem Statement

RSSP1 is a safety communication protocol widely used in railway signalling systems (IEC 62280). It provides authenticated, tamper-proof data transfer between safety-critical peers over untrusted networks.

However, local applications (simulators, test tools, HMI panels) typically speak plain UDP — they don't implement the RSSP1 stack. Writing a custom RSSP1 integration for each tool is impractical and error-prone.

**pi_forwarder** bridges this gap. It acts as a transparent proxy:

```
Local App (plain UDP)  ←→  pi_forwarder  ←→  Remote Peer (RSSP1 over UDP)
```

Local applications send and receive plain UDP datagrams. The forwarder handles all RSSP1 protocol concerns — authentication, sequence numbers, timestamps, cycle discipline — so local tools stay simple.

## Quick Start

### Prerequisites

- **CMake** 3.20+
- **C++17 compiler** (GCC 11+, Clang 14+, or Visual Studio 2022)
- **ASIO standalone** 1.38.0 (see below)

### 1. Get ASIO

Download [asio-1.38.0](https://github.com/chriskohlhoff/asio/releases/tag/asio-1-36-0) and unpack it so that `asio.hpp` is at:

```text
third-party/asio-1.38.0/include/asio.hpp
```

```bash
# On Linux / macOS
mkdir -p third-party
unzip asio-1.38.0.zip -d third-party/

# On Windows (PowerShell)
New-Item -ItemType Directory -Force third-party
Expand-Archive asio-1.38.0.zip -DestinationPath third-party
```

### 2. Build

```bash
cmake -S . -B build
cmake --build build
```

On Windows with Visual Studio, add `--config Release` for an optimized build:

```powershell
cmake --build build --config Release
```

### 3. Run

Edit `config/forwarder.json` with your RSSP1 parameters and peer addresses, then:

```bash
./build/forwarder                    # Linux / macOS
.\build\Debug\forwarder.exe          # Windows (Visual Studio)
```

## Configuration

All RSSP1 parameters and connection settings live in `config/forwarder.json`. A working example ships with the repo.

Each connection defines:

| Field | Purpose |
|-------|---------|
| `udp_channel` | Local-app UDP socket — bind address, port, and idle peer timeout |
| `rssp1_channels` | RSSP1 peer UDP channels — local/remote IP:port pairs for the safety protocol |

For a complete field reference and default values, see the [Design Decisions](AGENTS.md#design-decisions) in AGENTS.md.

## Project Docs

- **[AGENTS.md](AGENTS.md)** — architecture, processing model, threading model, code style, and design decisions.
