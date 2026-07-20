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

### 1. Third-Party Dependencies

The project vendors its dependencies under `third-party/`. Make sure these are present:

| Library | Version | Link |
|---------|---------|------|
| ASIO standalone | 1.38.0 | https://github.com/chriskohlhoff/asio/releases/tag/asio-1-38-0 |
| nlohmann/json | 3.12.0 | https://github.com/nlohmann/json/releases/tag/v3.12.0 |

ASIO should be unpacked so that `asio.hpp` is at:

```text
third-party/asio-1.38.0/include/asio.hpp
```

nlohmann/json is a single header — place `json.hpp` directly under `third-party/nlohmann/`.

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
