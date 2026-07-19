# Roadmap: UDP ↔ RSSP1 Forwarder

This roadmap covers the implementation from the current all-stub scaffold to a fully working forwarder. Each phase builds on the previous one and ends with a compilable, testable checkpoint.

---

## Phase 1 — Configuration, Logging & INI Generation

**Goal:** Parse `config/forwarder.json`, generate the RSSP1 INI in memory, and pass it to the RSSP1 stack at init. Add structured logging.

- [x] **1.1 — Add a logging utility** (`src/forwarder/log.hpp` / `log.cpp`)
  - A simple severity-leveled logger (trace, debug, info, warn, error) writing to stdout/stderr with timestamps.
  - All later components `#include "log.hpp"` and log through it — never call `GM_RSSP1_Log_Msg` directly from C++.

- [x] **1.2 — Implement `Config`** (`config.hpp` / `config.cpp`)
  - Parse `config/forwarder.json` (path from CLI argument) using `nlohmann/json` (single header, vendored under `third-party/`).
  - Populate these structs:

    ```
    pure_udp_layer:
      local_ip, local_port   → bind address for local app UDP socket
      peer_timeout_ms        → idle timeout before forgetting the learned peer

    rssp1_global:
      addr                   → this device's RSSP1 source address
      main_cycle_ms          → cycle period
      sys_chk                → system check words {A, B}
      keys                   → {data_ver, sinit, sid} × {A, B}

    rssp1_connections[]:
      addr                   → peer's RSSP1 dest address
      keys                   → peer's {data_ver, sinit, sid} × {A, B}
      udp_channels[]         → {local_ip, local_port, remote_ip, remote_port}
    ```

  - Apply defaults for all optional fields (see AGENTS.md Design Decisions).

- [x] **1.3 — Implement INI generator** (`src/forwarder/ini_generator.hpp` / `ini_generator.cpp`)
  - Takes the parsed `Config` struct and produces the INI string the RSSP1 module expects.
  - Sections: `[RSSP1_GLOBAL]`, `[CON_0]` (one per connection).
  - `connection_num` / `Max_ConnectNum` = length of `rssp1_connections`.
  - `com_type` hardcoded to `0` (UDP).
  - Hex values written with `0x` prefix.
  - Dump the generated INI to the log at debug level.

- [x] **1.4 — Update `main.cpp`** to accept an optional config file path argument.

**Checkpoint:** Build and run — the app parses the JSON, prints the loaded config, dumps the generated INI to the log, and exits.

---

## Phase 2 — ASIO I/O Skeleton

**Goal:** Bring up the `asio::io_context`, the local-app UDP socket (with auto-learn), and the RSSP1 peer UDP socket. Prove both can receive and send.

- [x] **2.1 — Implement `UdpSocket`** (`udp_socket.hpp` / `udp_socket.cpp`)
  - Constructor takes `asio::io_context&` and a local endpoint, opens and binds the socket.
  - `async_receive(callback)` — posts an async read, calls the callback with `(std::vector<std::uint8_t>, asio::ip::udp::endpoint)` on each datagram. Re-arms immediately.
  - `send_to(data, endpoint)` — async send to a remote endpoint.
  - RAII — destructor closes the socket.

- [x] **2.2 — Implement auto-learn on the local-app socket**
  - On the first received datagram, record the sender's `ip:port` as the remote peer. Log it prominently.
  - All subsequent inbound datagrams from other addresses are silently dropped.
  - Track last-received timestamp; if `peer_timeout_ms` elapses with no data, prune the peer and log at info level. The next sender becomes the new peer.
  - `send_to_peer(data)` — sends to the remembered peer (no-op if no peer established).

- [x] **2.3 — Start wiring `Forwarder`** (`forwarder.hpp` / `forwarder.cpp`)
  - Own an `asio::io_context`.
  - Own a `UdpSocket` for the local app (bound to `pure_udp_layer.local_ip:local_port`).
  - Own a `UdpSocket` for each RSSP1 peer channel (bound to each `udp_channels[].local_ip:local_port`).
  - Start async receive loops on all sockets — for now, just log received data.
  - `run()` calls `io_context.run()`.

**Checkpoint:** Send a UDP packet to the local-app socket — the forwarder logs the learned peer. Send to an RSSP1 peer socket — same. Both sockets stay alive and keep receiving.

---

## Phase 3 — Cycle Timer & Queue Infrastructure

**Goal:** Implement the strict cycle with the steady timer plus the two decoupling queues. After this phase the timing and data plumbing are in place; only the RSSP1 calls are missing.

- [x] **3.1 — Add queue types** (new header `src/forwarder/queues.hpp`)
  - `rx_frame_queue` — a `std::deque` of `{std::vector<std::uint8_t> raw_bytes}` representing RSSP1 frames from the peer.
  - `tx_payload_queue` — a `std::deque` of `{std::vector<std::uint8_t> payload, asio::ip::udp::endpoint target}` from local apps.
  - Since everything is single-threaded, plain `std::deque` (no mutex) is sufficient.

- [x] **3.2 — Add the cycle timer to `Forwarder`**
  - Create an `asio::steady_timer` that fires every `cycle_period_ms`.
  - The timer handler calls two private methods in order: `do_receive_pass()` then `do_send_pass()`, then re-arms the timer.
  - For now, `do_receive_pass()` and `do_send_pass()` are empty stubs that log "receive pass" / "send pass".

- [x] **3.3 — Redirect UDP receive handlers to push into queues**
  - RSSP1 peer socket handler → push raw bytes into `rx_frame_queue`.
  - Local-app socket handler → push `{payload, sender_endpoint}` into `tx_payload_queue`.
  - Both handlers immediately re-post `async_receive()` to keep the socket alive.

**Checkpoint:** The timer fires on schedule, incoming UDP data lands in the correct queues, and the app logs queue depths in each pass.

---

## Phase 4 — RSSP1 Adapter (C Wrapper)

**Goal:** Wrap the critical `GM_RSSP1_*` C API behind `Rssp1Adapter` so no other C++ file touches `extern "C"` RSSP1 headers.

**Prerequisite:** The vendored RSSP1 C sources are always compiled — no special flag needed.

- [x] **4.1 — Implement `Rssp1Adapter`** (`rssp1_adapter.hpp` / `rssp1_adapter.cpp`)
  - Include all needed RSSP1 C headers inside `extern "C"` blocks in the `.cpp` file only.
  - Public methods matching the cycle operations:
    - `init(const std::string& ini_content)` — passes the generated INI string to `GM_RSSP1_APP_Interface_Init(GetAbas, ini_content.data(), GM_RSSP1_FALSE, NULL)`.
    - `rcv_com_interface(const std::vector<std::uint8_t>& frame)` — feeds a raw frame into the stack via `GM_RSSP1_RCV_com_Interface()`.
    - `receive_pass()` — calls `GM_RSSP1_APP_Interface_RxPrc()`.
    - `drain_received(std::vector<ReceivedPayload>& out)` — loops `GM_RSSP1_APP_Interface_Rcv_App_Dat()` until no more data, collecting `{data, source_addr}` for each payload.
    - `send_app_data(const std::vector<std::uint8_t>& data, std::uint16_t dest_addr)` — calls `GM_RSSP1_APP_Interface_Send_App_Dat()`.
    - `send_pass()` — calls `GM_RSSP1_APP_Interface_TxPrc()`.
    - `drain_to_send(std::vector<std::vector<std::uint8_t>>& out)` — loops `GM_RSSP1_SND_com_Interface()` until no more frames, collecting raw bytes for each.
    - `connection_state()` — queries `GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info()` etc.
    - `set_vsn_callback(...)` — registers the VSN (timestamp) callback the stack requires.
  - Define helper types: `ReceivedPayload { std::vector<std::uint8_t> data; std::uint32_t source_addr; }`.
  - Provide stub implementations of any callbacks the C stack needs (e.g. `GM_RSSP_GET_ABAS_FUN`, `VSN_GET_CALLBACK_FUN`).

- [x] **4.2 — Handle RSSP1 build issues**
  - The vendored C code may have minor build errors under MSVC (e.g. `#pragma pack`, non-standard extensions, or missing includes). Fix only build errors; do not refactor.
  - If the C code uses `GM_RSSP1_DISABLE_LOCK`, ensure that macro is set (we are single-threaded).

**Checkpoint:** Compiles and links. The adapter initializes the RSSP1 stack successfully at startup (log the init result).

---

## Phase 5 — Per-Connection Config Refactor

**Goal:** Restructure `config/forwarder.json` so each RSSP1 connection owns its local-app UDP channel. This eliminates the single global `pure_udp_layer` and moves it into each connection as `udp_channel`, enabling multi-peer setups where different local applications talk to different RSSP1 peers.

### Config schema changes

| Old (global) | New (per-connection) |
|---|---|
| `pure_udp_layer` (top-level) | `udp_channel` (inside each `connections[]` entry) |
| `rssp1_global` | `rssp1_params` |
| `rssp1_connections` | `connections` |
| `udp_channels` (per-connection) | `rssp1_channels` (per-connection) |

### Tasks

- [ ] **5.1 — Update `config.hpp` / `config.cpp`**
  - Replace `PureUdpLayer` struct with `UdpChannel` struct (same fields: `local_ip`, `local_port`, `peer_timeout_ms`).
  - Rename `Rssp1Global` → `Rssp1Params`.
  - Move `UdpChannel` into `Connection` as field `udp_channel`.
  - Rename `rssp1_connections` → `connections`, `udp_channels` → `rssp1_channels`.
  - Update JSON key parsing (`from_json`) accordingly.

- [ ] **5.2 — Update `ini_generator.cpp`**
  - Update section key name from `rssp1_global` / `rssp1_connections` to match renamed struct fields.

- [ ] **5.3 — Update `forwarder.hpp` / `forwarder.cpp`**
  - Create one local-app `UdpSocket` per connection (was: one global `local_socket_`).
  - Each connection owns its own auto-learn peer state.
  - RSSP1 peer sockets are already per-connection; no change needed there.

- [ ] **5.4 — Simplify `address_map.hpp` / `address_map.cpp`**
  - With per-connection local sockets, the mapping becomes 1:1 — each connection's `udp_channel` maps to its `rssp1_channels` and vice versa.
  - The `AddressMap` can be reduced to a simple lookup.

**Checkpoint:** Forwarder binds one local UDP socket per connection. Sending to a local peer on connection 0 routes to that connection's RSSP1 peer.

---

## Phase 6 — Address Map

**Goal:** Route between the local UDP peer and the correct RSSP1 connection. Trivial for single-connection setups; required when multiple RSSP1 peers are configured.

- [ ] **6.1 — Implement `AddressMap`** (`address_map.hpp` / `address_map.cpp`)
  - Single-connection case (common): all local-app traffic maps to the sole `connections[0].addr`, and all RSSP1 RX payloads map to the learned local UDP peer. Essentially a no-op.
  - Multi-connection case (future): each RSSP1 connection's `addr` maps to a specific local UDP port or a per-connection auto-learned peer.
  - `resolve_udp_to_rssp1(const asio::ip::udp::endpoint&)` → returns `std::optional<std::uint16_t>`.
  - `resolve_rssp1_to_udp(std::uint32_t source_addr)` → returns `std::optional<asio::ip::udp::endpoint>`.

**Checkpoint:** Unit-testable in isolation — supply endpoints, verify correct mappings.

---

## Phase 7 — Data Paths: UDP↔RSSP1

**Goal:** Wire `UdpToRsp1` and `Rssp1ToUdp` to connect the queues, the adapter, the address map, and the UDP sockets so data actually flows end-to-end.

- [ ] **7.1 — Implement `Rssp1ToUdp`** (receive path — RSSP1 peer → local app)
  - Takes references to `rx_frame_queue`, `Rssp1Adapter`, `AddressMap`, and the local-app `UdpSocket`.
  - `process_receive_pass()`:
    1. Dequeue each raw frame from `rx_frame_queue` (with its `src_ip`, `src_port` from the UDP header).
    2. Feed to `adapter.rcv_com_interface(buf, len, src_ip, src_port, 0, 0, 1)`.
    3. If it returns `true`: call `adapter.cfm_proc_recv()` then `adapter.sfm_proc_recv()`.
    4. Loop `adapter.drain_received()` → each payload has a **6-byte header** — skip it, pass `buf+6` / `len-6` as the actual app data.
    5. Resolve `src_addr` → local UDP endpoint via `AddressMap`, then `UdpSocket::send_to_peer(data)`.
    (The pass runs even if the queue is empty — the proc functions must still be called to advance the stack.)

- [ ] **7.2 — Implement `UdpToRsp1`** (send path — local app → RSSP1 peer)
  - Takes references to `tx_payload_queue`, `Rssp1Adapter`, `AddressMap`, and the RSSP1 peer `UdpSocket`.
  - `process_send_pass()`:
    1. Dequeue all pending payloads from `tx_payload_queue`.
    2. For each: `AddressMap::resolve_udp_to_rssp1()` → `dest_addr`, then `adapter.send_app_data(data, len, dest_addr)`.
    3. Call `adapter.sfm_proc_send()` then `adapter.cfm_proc_send()` (runs every cycle — the stack emits periodic SSE/SSR/RSD frames).
    4. Call `adapter.drain_to_send()` → returns `{buf, len, ip, port, index, chn_index}`. The `ip` is in **network byte order**.
    5. Send the raw frame via `UdpSocket::send_to(buf, len, resolved_ip, port)`.

- [ ] **7.3 — Wire everything in `Forwarder`**
  - `do_receive_pass()` delegates to `rssp1_to_udp.process_receive_pass()`.
  - `do_send_pass()` delegates to `udp_to_rssp1.process_send_pass()`.
  - Construct all components in `Forwarder::Forwarder(const Config&)` with the correct dependency order.

**Checkpoint:** Full end-to-end data flow — send UDP to the local-app port, it surfaces on the RSSP1 peer socket, and vice versa.

---

## Phase 8 — Robustness & Edge Cases

**Goal:** Handle the real world — connection state changes, errors, idle cycles, and config reload.

- [ ] **8.1 — Connection state monitoring**
  - Log connection state changes from the RSSP1 adapter (SFM connection state reports).
  - If the RSSP1 connection is down, optionally buffer or drop UDP payloads rather than losing them silently.

- [ ] **8.2 — Error handling**
  - RSSP1 API errors → log and continue (never crash the forwarder).
  - UDP send/receive errors → log and re-arm (transient network issues recover).
  - Queue overflow → log a warning and drop oldest or newest entry (pick one policy).

- [ ] **8.3 — Graceful shutdown**
  - `Forwarder::~Forwarder()` or a `stop()` method:
    - Cancel the cycle timer.
    - Close both UDP sockets.
    - Call RSSP1 teardown if the stack provides one.
    - `io_context.stop()`.

- [ ] **8.4 — Signal handling (optional but recommended)**
  - Handle `SIGINT` / `SIGTERM` (or `SetConsoleCtrlHandler` on Windows) to trigger graceful shutdown.

**Checkpoint:** The forwarder survives network glitches, logs problems clearly, and shuts down cleanly.

---

## Phase 9 — Testing

**Goal:** Add automated tests that validate correctness without needing real RSSP1 peer hardware.

- [ ] **9.1 — Unit tests for `Config`** — parse a known JSON string, verify fields.
- [ ] **9.2 — Unit tests for `AddressMap`** — test resolve in both directions, missing entries, defaults.
- [ ] **9.3 — Integration test harness**
  - Two forwarder instances on different loopback ports talking to each other.
  - Send known UDP payloads, verify they arrive intact at the far side.
  - Measure end-to-end latency.

- [ ] **9.4 — RSSP1 adapter mock** (optional, for testing data paths without the C stack)
  - An `Rssp1Adapter` interface with a mock implementation that echoes data through the cycle, so the queue/pipeline logic can be tested without the full RSSP1 C library.

**Checkpoint:** `ctest` (or `cmake --build` + test runner) passes all tests.

---

## Phase 10 — Packaging & Documentation

- [ ] **10.1 — Polish README.md** with usage examples, config file reference, and build instructions.
- [ ] **10.2 — Keep `config/forwarder.json` as the documented example** — all fields shown with their defaults in comments.
- [ ] **10.3 — Consider CI** (GitHub Actions or similar) to build on Windows and run tests.
- [ ] **10.4 — Add a `--version` flag** and print the RSSP1 library version string (`GM_RSSP1_TABLE_Ver`) at startup.

---

## Dependency Order

```
Phase 1 (Config + Logging)
  └─► Phase 2 (ASIO I/O Skeleton)
       └─► Phase 3 (Cycle Timer + Queues)
            └─► Phase 4 (RSSP1 Adapter)
                 └─► Phase 5 (Per-Connection Config)
                      └─► Phase 6 (Address Map)
                           └─► Phase 7 (Data Paths: UDP ↔ RSSP1)
                                └─► Phase 8 (Robustness)
                                     └─► Phase 9 (Testing)
                                          └─► Phase 10 (Packaging)
```

Phases 4, 5, and 6 can be worked on in parallel once Phase 3 is done.

---

## Quick Reference: Build Commands

```bash
cmake --preset vs2022-debug
cmake --build out/build/vs2022-debug
```
