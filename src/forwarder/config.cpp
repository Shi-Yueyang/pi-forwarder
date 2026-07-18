#include "config.hpp"
#include "log.hpp"

#include "json.hpp" // nlohmann/json single header

#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace forwarder {

// ============================================================================
//  File-static helpers
// ============================================================================

/// Parse a hex string like "0xae390b5a" to uint32_t.
/// Throws std::invalid_argument on bad input.
static std::uint32_t parse_hex32(const std::string& s, const std::string& field_name)
{
    if (s.size() < 3 || s[0] != '0' || (s[1] != 'x' && s[1] != 'X')) {
        throw std::invalid_argument(
            field_name + ": expected hex string with 0x prefix, got '" + s + '\'');
    }
    try {
        return static_cast<std::uint32_t>(std::stoul(s, nullptr, 16));
    } catch (const std::exception& e) {
        throw std::invalid_argument(
            field_name + ": invalid hex value '" + s + "': " + e.what());
    }
}

/// Parse a JSON object of form {"A": "0x...", "B": "0x..."} into a KeyVal.
static KeyVal parse_keyval(const nlohmann::json& j, const std::string& context)
{
    if (!j.is_object()) {
        throw std::runtime_error(context + ": expected an object with A and B keys");
    }
    if (!j.contains("A") || !j.contains("B")) {
        throw std::runtime_error(context + ": missing required A or B key");
    }
    KeyVal kv;
    kv.a = parse_hex32(j["A"].get<std::string>(), context + ".A");
    kv.b = parse_hex32(j["B"].get<std::string>(), context + ".B");
    return kv;
}

/// Parse a JSON object of form {"A": "0x...", "B": "0x..."} into a SysChk.
static SysChk parse_syschk(const nlohmann::json& j, const std::string& context)
{
    if (!j.is_object()) {
        throw std::runtime_error(context + ": expected an object with A and B keys");
    }
    if (!j.contains("A") || !j.contains("B")) {
        throw std::runtime_error(context + ": missing required A or B key");
    }
    SysChk sc;
    sc.a = parse_hex32(j["A"].get<std::string>(), context + ".A");
    sc.b = parse_hex32(j["B"].get<std::string>(), context + ".B");
    return sc;
}

// ============================================================================
//  Public API
// ============================================================================

Config parse_config_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_config_string(buffer.str());
}

Config parse_config_string(const std::string& json_str)
{
    // ---- Parse JSON ----
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(json_str);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }

    Config config;

    // ---- 1. log_level (top-level, optional) ----
    if (j.contains("log_level") && j["log_level"].is_string()) {
        config.log_level = log_level_from_string(j["log_level"].get<std::string>());
    }

    // ---- 2. pure_udp_layer (optional section) ----
    if (j.contains("pure_udp_layer") && j["pure_udp_layer"].is_object()) {
        const auto& udp = j["pure_udp_layer"];
        auto& pu = config.pure_udp_layer;
        pu.local_ip       = udp.value("local_ip",       pu.local_ip);
        pu.local_port     = udp.value("local_port",     pu.local_port);
        pu.peer_timeout_ms = udp.value("peer_timeout_ms", pu.peer_timeout_ms);
    }

    // ---- 3. rssp1_global (required section) ----
    if (!j.contains("rssp1_global")) {
        throw std::runtime_error("Missing required top-level field: 'rssp1_global'");
    }
    {
        const auto& g = j["rssp1_global"];
        auto& rg = config.rssp1_global;

        rg.addr          = parse_hex32(g.at("addr").get<std::string>(), "rssp1_global.addr");
        rg.main_cycle_ms = g.value("main_cycle_ms", 200);

        // sys_chk (required)
        if (g.contains("sys_chk")) {
            rg.sys_chk = parse_syschk(g["sys_chk"], "rssp1_global.sys_chk");
        } else {
            throw std::runtime_error("Missing required field: rssp1_global.sys_chk");
        }

        // keys (required)
        if (!g.contains("keys")) {
            throw std::runtime_error("Missing required field: rssp1_global.keys");
        }
        {
            const auto& gk = g["keys"];
            const std::string ctx = "rssp1_global.keys";
            rg.keys.data_ver = parse_keyval(gk.at("data_ver"), ctx + ".data_ver");
            rg.keys.sinit    = parse_keyval(gk.at("sinit"),    ctx + ".sinit");
            rg.keys.sid      = parse_keyval(gk.at("sid"),      ctx + ".sid");
        }

        // Optional queue_sizes
        if (g.contains("queue_sizes") && g["queue_sizes"].is_object()) {
            const auto& q = g["queue_sizes"];
            rg.queue_sizes.sfm_u2l_per_connection = q.value("sfm_u2l_per_connection", 80);
            rg.queue_sizes.sfm_l2u_per_connection = q.value("sfm_l2u_per_connection", 80);
            rg.queue_sizes.cfm_u2l_per_connection = q.value("cfm_u2l_per_connection", 80);
            rg.queue_sizes.cfm_l2u_per_connection = q.value("cfm_l2u_per_connection", 80);
        }

        rg.usrdata_all0_size = g.value("usrdata_all0_size", 0);
    }

    // ---- 4. rssp1_connections (required, may be empty array) ----
    if (!j.contains("rssp1_connections")) {
        throw std::runtime_error("Missing required top-level field: 'rssp1_connections'");
    }
    if (!j["rssp1_connections"].is_array()) {
        throw std::runtime_error("'rssp1_connections' must be an array");
    }

    for (size_t i = 0; i < j["rssp1_connections"].size(); ++i) {
        const auto& c = j["rssp1_connections"][i];
        Rssp1Connection conn;
        const std::string conn_label = "rssp1_connections[" + std::to_string(i) + "]";

        // addr (required)
        conn.addr = static_cast<std::uint16_t>(
            parse_hex32(c.at("addr").get<std::string>(), conn_label + ".addr"));

        // keys (required)
        if (!c.contains("keys")) {
            throw std::runtime_error("Missing required field: " + conn_label + ".keys");
        }
        {
            const auto& ck = c["keys"];
            const std::string kctx = conn_label + ".keys";
            conn.keys.data_ver = parse_keyval(ck.at("data_ver"), kctx + ".data_ver");
            conn.keys.sinit    = parse_keyval(ck.at("sinit"),    kctx + ".sinit");
            conn.keys.sid      = parse_keyval(ck.at("sid"),      kctx + ".sid");
        }

        // Optional connection-level fields
        conn.fsfb_comm_cycle_ms   = c.value("fsfb_comm_cycle_ms",   -1);
        conn.local_node_cycle_ms  = c.value("local_node_cycle_ms",  -1);
        conn.num_data_ver         = c.value("num_data_ver",          1);
        conn.is_fix_node          = c.value("is_fix_node",          true);
        conn.remote_dev_is_A      = c.value("remote_dev_is_A",      true);
        conn.enable_crscd_pack    = c.value("enable_crscd_pack",    false);
        conn.enable_per_channel_fsfb = c.value("enable_per_channel_fsfb", false);
        conn.chn_apply_fsfb_id    = c.value("chn_apply_fsfb_id",    4001);
        conn.l2u_queue_size       = c.value("l2u_queue_size",       2);

        // Optional timing sub-object
        if (c.contains("timing") && c["timing"].is_object()) {
            const auto& t = c["timing"];
            conn.timing.delta_time     = t.value("delta_time",     5);
            conn.timing.life_time      = t.value("life_time",      5);
            conn.timing.delay_time     = t.value("delay_time",     5);
            conn.timing.tolerate_cycle = t.value("tolerate_cycle", 6);
        }

        // udp_channels (required)
        if (!c.contains("udp_channels") || !c["udp_channels"].is_array()) {
            throw std::runtime_error(
                "Missing or invalid 'udp_channels' in " + conn_label);
        }
        for (size_t ch = 0; ch < c["udp_channels"].size(); ++ch) {
            const auto& uc = c["udp_channels"][ch];
            UdpChannel channel;
            channel.local_ip      = uc.value("local_ip",   std::string("127.0.0.1"));
            channel.local_port    = uc.value("local_port",  0);
            channel.remote_ip     = uc.value("remote_ip",  std::string("127.0.0.1"));
            channel.remote_port   = uc.value("remote_port", 0);
            channel.recv_queue_size = uc.value("recv_queue_size", 5);
            channel.send_queue_size = uc.value("send_queue_size", 5);
            conn.udp_channels.push_back(std::move(channel));
        }

        config.rssp1_connections.push_back(std::move(conn));
    }

    return config;
}

} // namespace forwarder
