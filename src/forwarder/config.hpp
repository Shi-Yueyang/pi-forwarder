#pragma once

// Runtime configuration: UDP endpoints, RSSP1 peer mappings, and role.
// All structs use in-class default initializers matching AGENTS.md defaults.

#include <cstdint>
#include <string>
#include <vector>

#include "log.hpp"

namespace forwarder {

// ---- Hex value pair (A/B system check words) ----
struct SysChk {
    std::uint32_t a = 0;
    std::uint32_t b = 0;
};

// ---- Key pair for data_ver, sinit, sid ----
struct KeyVal {
    std::uint32_t a = 0;
    std::uint32_t b = 0;
};

struct Keys {
    KeyVal data_ver;
    KeyVal sinit;
    KeyVal sid;
};

// ---- Queue sizes (optional section, with defaults) ----
struct QueueSizes {
    int sfm_u2l_per_connection = 80;
    int sfm_l2u_per_connection = 80;
    int cfm_u2l_per_connection = 80;
    int cfm_l2u_per_connection = 80;
};

// ---- Timing parameters per connection (optional section) ----
struct Timing {
    int delta_time     = 5;
    int life_time      = 5;
    int delay_time     = 5;
    int tolerate_cycle = 6;
};

// ---- Local-app UDP channel for a single RSSP1 connection ----
struct UdpChannel {
    std::string  local_ip   = "127.0.0.1";
    std::uint16_t local_port = 0;
    int peer_timeout_ms = 30000;
};

// ---- A single RSSP1 peer channel within a connection ----
struct Rssp1Channel {
    std::string  local_ip;
    std::uint16_t local_port  = 0;
    std::string  remote_ip;
    std::uint16_t remote_port = 0;
    int recv_queue_size = 5;
    int send_queue_size = 5;
};

// ---- A single RSSP1 peer connection ----
struct Connection {
    std::uint16_t addr = 0;
    Keys                    keys;

    // Local-app socket config (one per connection)
    UdpChannel udp_channel;

    // RSSP1 peer channels
    std::vector<Rssp1Channel> rssp1_channels;

    // Optional fields. Sentinel -1 for cycle fields means "use local_rssp1_params.main_cycle_ms".
    int  fsfb_comm_cycle_ms   = -1;
    int  local_node_cycle_ms  = -1;
    int  num_data_ver         = 1;
    bool is_fix_node           = true;
    bool remote_dev_is_A       = true;
    Timing timing;
    bool enable_crscd_pack         = false;
    bool enable_per_channel_fsfb  = false;
    int  chn_apply_fsfb_id        = 4001;
    int  l2u_queue_size           = 2;
};

// ---- Global RSSP1 stack parameters ----
struct Rssp1Params {
    std::uint16_t addr = 0;
    int     main_cycle_ms = 200;
    SysChk  sys_chk;
    Keys    keys;
    QueueSizes queue_sizes;
    int     usrdata_all0_size = 0;
};

// ---- Top-level configuration ----
struct Config {
    LogLevel log_level = LogLevel::Info; // resolved from string after parsing
    Rssp1Params                     local_rssp1_params;
    std::vector<Connection>         connections;
};

// ---- Parsing functions ----

/// Parse a JSON config file from disk.
/// Throws std::runtime_error on failure (file not found, parse error, missing required field).
Config parse_config_file(const std::string& path);

/// Parse a JSON string directly (useful for testing).
/// Throws std::runtime_error on failure.
Config parse_config_string(const std::string& json_str);

} // namespace forwarder
