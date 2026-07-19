# Project Guidelines

## Overview

This workspace implements a **UDP ↔ RSSP1 forwarder**: a C++ gateway that receives plain UDP packets from local applications, forwards them over the RSSP1 safety protocol, and extracts received RSSP1 payloads back to UDP.

The project reuses the existing C RSSP1 stack under `src/GM_RSSPI_V2.0.14/` and wraps it with a modern C++ platform layer built on **ASIO standalone**.

## Architecture

### Directory Layout

```text
pi_forwarder/
├── CMakeLists.txt                # Build definition
├── CMakePresets.json             # VS2022 configure & build presets
├── config/
│   └── forwarder.json            # Runtime configuration template
├── tests/
│   └── .gitkeep                  # (no tests yet)
├── third-party/
│   └── asio-1.36.0/              # Vendored standalone ASIO headers
├── tools/
│   └── convert_gb2312_to_utf8.py
└── src/
    ├── main.cpp                  # Entry point
    ├── forwarder/                # C++ forwarder implementation
    │   ├── forwarder.{hpp,cpp}   # ✅ Top-level orchestrator (owns io_context, sockets, peer state)
    │   ├── config.{hpp,cpp}      # ✅ JSON config parser (nlohmann/json, 7 structs)
    │   ├── log.{hpp,cpp}         # ✅ Structured console logger (5 levels, timestamps, macros)
    │   ├── ini_generator.{hpp,cpp}  # ✅ JSON → RSSP1 INI string generator
    │   ├── udp_socket.{hpp,cpp}  # ✅ RAII ASIO UDP socket (async recv loop, fire-and-forget send)
    │   ├── rssp1_adapter.{hpp,cpp}  # ⬜ RSSP1 C API wrapper stub (Phase 4)
    │   ├── udp_to_rssp1.{hpp,cpp}   # ⬜ UDP → RSSP1 send path stub (Phase 6)
    │   ├── rssp1_to_udp.{hpp,cpp}   # ⬜ RSSP1 → UDP receive path stub (Phase 6)
    │   └── address_map.{hpp,cpp}    # ⬜ Endpoint mapping stub (Phase 5)
    └── GM_RSSPI_V2.0.14/        # Vendored C RSSP1 library (do not refactor)
        ├── GM_RSSP1_APP_Interface.{h,c}  # Application-layer interface
        ├── GM_RSSP1_CFM_Interface.{h,c}  # Communication layer (raw frames)
        ├── GM_RSSP1_Lib_Def.h            # Config structs & type definitions
        ├── CFM/         # Communication Functional Module (7 files)
        ├── SFM/         # Safety Functional Module (8 files)
        └── common/      # Shared utilities: CRC, hash, VSN, mutex, etc. (12 files)
```

### Current Status

**Phase 1 (Config, Logging, INI Generation) — COMPLETE.** The forwarder parses `config/forwarder.json` at startup, configures structured console logging, generates the RSSP1 INI string in memory, and logs it at DEBUG level.

**Phase 2 (ASIO I/O Skeleton) — COMPLETE.** The forwarder owns an `asio::io_context`, binds local-app UDP sockets (with auto-learn peer logic) and RSSP1 peer channel UDP sockets, and runs the event loop. Both socket types receive and log incoming datagrams.

**Phase 3 (Cycle Timer & Queues) — COMPLETE.** An `asio::steady_timer` drives a strict receive-pass-then-send-pass cycle. Two decoupling queues (`rx_frame_queue`, `tx_payload_queue`) sit between the async I/O handlers and the cycle processing.

**Phase 4 (RSSP1 Adapter) — COMPLETE.** The `Rssp1Adapter` wraps all `GM_RSSP1_*` C API calls behind a clean C++ interface. The vendored RSSP1 C sources compile into a static library. The adapter initializes the stack at startup and advances the VSN each cycle.

**Remaining (Phases 5–10):** `address_map`, `udp_to_rssp1`, and `rssp1_to_udp` are still bare stubs. Phase 5 refactors the config to per-connection UDP channels.

### Processing Model (Design): Strict Cycle

The RSSP1 stack is **cycle-driven, not event-driven**. It runs in a strict periodic cycle: in every cycle the protocol stack must perform exactly one receive pass and one send pass, in that order. No `GM_RSSP1_*` API is ever called from an ASIO I/O completion handler; handlers only enqueue data, and all RSSP1 processing happens inside the cycle.

Queues decouple the asynchronous I/O from the synchronous cycle:

- **rx frame queue**: raw RSSP1 frames received from the peer, waiting to be fed into the next receive pass.
- **tx payload queue**: UDP payloads from local applications, waiting to be submitted in the next send pass.

An `asio::steady_timer` fires once per cycle period and executes:

#### 1. Receive pass (once per cycle)

For each raw frame dequeued from the rx frame queue:

1. Feed the frame into the stack:
   `GM_RSSP1_RCV_com_Interface(buf, recv_len, src_ip, src_port, 0, 0, 1)`
   - `src_ip` — the sender's IP in **network byte order** (`sockaddr_in.sin_addr.s_addr`).
   - `src_port` — the sender's port in **host byte order** (`ntohs(sockaddr_in.sin_port)`).
   - `index=0`, `chn_index=0`, `mode=1` (mode 1 = normal data input).
2. If `RCV_com_Interface` returns `true` (the frame matched a configured connection):
   `GM_RSSP1_CFM_Interface_Proc_Recv()` then `GM_RSSP1_SFM_Interface_Proc_Recv()`.
   (These can also be called via the convenience wrapper `GM_RSSP1_APP_Interface_RxPrc()`, which adds SAVING_MODE bookkeeping before the same two calls.)
3. Extract the verified application payload:
   `GM_RSSP1_APP_Interface_Rcv_App_Dat(buf, &src_addr, &len, &remaining_count)`
   - Returns `> 0`: a payload is available. The payload in `buf` has a **6-byte header** that must be stripped — the actual application data starts at `buf + 6` with length `len - 6`.
   - Returns `0`: a non-application message (connection state, warning).
   - Returns `< 0`: no more messages.
   - Loop until the return is ≤ 0 to drain all queued payloads.
4. `address_map` resolves `src_addr` (the source SaCEPID) to a local UDP endpoint; `udp_socket` sends the payload data (without the 6-byte header) to the local application.

> **Note:** The receive pass must run even if the rx frame queue is empty — call `RCV_com_Interface` with `recv_len=0` (or skip and just call the proc functions) so the stack's internal timers and state machines advance.

#### 2. Send pass (once per cycle)

1. Dequeue pending UDP payloads from the tx payload queue.
2. For each payload: `address_map` resolves the UDP destination to an RSSP1 `dest_addr` (SaCEPID), then:
   `GM_RSSP1_APP_Interface_Send_App_Dat(data, len, dest_addr)`
   The raw application data is passed directly — the stack adds its own 6-byte header internally.
3. Run the layer send processing:
   `GM_RSSP1_SFM_Interface_Proc_Send()` then `GM_RSSP1_CFM_Interface_Proc_Send()`.
   (Or equivalently the wrapper `GM_RSSP1_APP_Interface_TxPrc()`.)
   **This must run every cycle even if no application data was submitted** — the stack needs to emit periodic protocol frames (SSE, SSR, RSD).
4. Drain the outbound frame:
   `GM_RSSP1_SND_com_Interface(buf, &len, &ip, &port, &index, &chn_index)`
   - Returns `true` when a frame is ready. The destination `ip` is in **network byte order** (use directly with `inet_ntoa` or `asio::ip::address_v4` constructor from `ntohl`-ed value).
   - Returns `false` when no frame is pending.
   - Call once per send pass (the stack emits at most one frame per cycle per connection).
5. Hand the raw frame to `udp_socket` for transmission to the resolved `ip:port` (the RSSP1 peer).

### Threading Model (Design)

**The entire program is single-threaded.** The main thread runs `asio::io_context::run()` and nothing else; no additional threads are created anywhere.

- All I/O (UDP receive/send) and all `GM_RSSP1_*` calls happen on this one thread.
- The cycle timer handler performs the receive pass and send pass; UDP receive handlers only push into the rx frame / tx payload queues.
- Because everything runs on one thread, no locking, strands, or the library's mutex hooks are needed.

## Code Style

- C++17 or later.
- Use RAII wrappers for all resources (sockets, timers, memory).
- Prefer `std::vector<std::uint8_t>` for byte buffers.
- Keep the RSSP1 C API wrapped behind `rssp1_adapter`; do not spread `extern "C"` includes across the codebase.
- Use `asio::ip::udp::endpoint` for UDP addressing.
- Naming: `snake_case` for files, functions, and variables; `PascalCase` for classes and structs.

## Build and Test

- Build system: **CMake** 3.20+.
- Networking library: **ASIO standalone** (vendored under `third-party/asio-1.36.0/`). Discovered via candidate directory search; `ASIO_STANDALONE` and `_WIN32_WINNT=0x0A00` are defined on the `asio` INTERFACE target.
- The RSSP1 C sources live under `src/GM_RSSPI_V2.0.14/` and are always compiled into a `rssp1` static library.
- The `forwarder` executable links `asio` and `rssp1`, and on Windows also links `ws2_32` and `mswsock`.
- **CMakePresets.json** provides VS2022 configure & build presets (debug, release).

Typical commands:

```bash
# Configure
cmake --preset vs2022-debug

# Build
cmake --build out/build/vs2022-debug
```

Tests live under `tests/` but no test code exists yet.

## Conventions

- **Do not touch any code under `src/GM_RSSPI_V2.0.14/` except to fix build errors.** Treat it as a vendored third-party library. Do not refactor, reformat, translate comments, or change its build configuration.
- All RSSP1 C headers must be included inside `extern "C"` blocks in `rssp1_adapter`.
- Configuration files live under `config/` and are loaded at startup.
- Use `asio::steady_timer` for the periodic RSSP1 processing cycle, not `std::this_thread::sleep_for`.
- Log through a single logging abstraction in `forwarder/`; do not call `GM_RSSP1_Log_Msg` directly from C++ code.
- The `tools/` directory contains helper scripts (e.g. GB2312→UTF-8 conversion for working with the RSSP1 C source).

## Design Decisions

Documenting how the forwarder behaves internally. When modifying any of these behaviors, update this section.

### Log level

`log_level` is a top-level field. Accepted values: `"trace"`, `"debug"`, `"info"`, `"warn"`, `"error"`. Default: `"info"`. Controls both the forwarder's own logging and the RSSP1 stack's internal log level (`GM_RSSP1_Set_LogLevel`).

### Configuration: JSON → internal INI generation

`config/forwarder.json` is the single source of truth. At startup the program generates the RSSP1 INI content in memory and passes it to `GM_RSSP1_APP_Interface_Init()` with `is_path=false`. The INI is an internal serialization detail — operators never see it. If a future RSSP1 library version adds new INI keys, the generator must be updated. The generated INI is dumped to the log at DEBUG level for debugging.

- `connection_num` and `Max_ConnectNum` in the generated INI are derived from the length of the `connections` array — not a separate field in the JSON.
- `queue_sizes` is optional. Defaults: `sfm_u2l_per_connection=80`, `sfm_l2u_per_connection=80`, `cfm_u2l_per_connection=80`, `cfm_l2u_per_connection=80`.
- `usrdata_all0_size` is optional. Default: `0`.
- Per connection: `fsfb_comm_cycle_ms` and `local_node_cycle_ms` are optional. Default: `main_cycle_ms` from `rssp1_params`.
- Per connection: `num_data_ver` (default `1`), `is_fix_node` (default `true`), `remote_dev_is_A` (default `true`) are optional.
- Per connection: `timing` is optional. Defaults: `delta_time=5`, `life_time=5`, `delay_time=5`, `tolerate_cycle=6`.
- Per connection: `enable_crscd_pack` (default `false`), `enable_per_channel_fsfb` (default `false`), `chn_apply_fsfb_id` (default `4001`), `l2u_queue_size` (default `2`) are optional.
- Per RSSP1 channel: `recv_queue_size` (default `5`), `send_queue_size` (default `5`) are optional.

### UDP layer: per-connection auto-learn remote peer

Each connection has a `udp_channel` section that configures a local UDP socket for talking to the local application. The forwarder remembers the first UDP sender as the remote peer for the lifetime of the process.

- On the first received UDP datagram, the sender's `ip:port` is recorded and logged prominently.
- All subsequent outbound UDP payloads go to that remembered peer.
- Inbound UDP from any other address is silently dropped after the peer is locked in.
- If no local peer is established when RSSP1 data arrives, the RX payload is dropped and a warning is logged.
- If no UDP datagram is received from the remembered peer within `peer_timeout_ms`, the peer is pruned and an info log is emitted. The forwarder is then ready to learn a new peer on the next datagram.
