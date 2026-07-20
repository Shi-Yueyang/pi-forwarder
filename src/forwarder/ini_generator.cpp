#include "ini_generator.hpp"
#include "config.hpp"

#include <iomanip>
#include <sstream>

namespace forwarder {

namespace {

/// Write a 32-bit hex key in 0x%08X format.
void write_hex32(std::ostringstream& ini, const char* key, std::uint32_t val)
{
    ini << key << "=0x" << std::hex << std::uppercase
        << std::setw(8) << std::setfill('0') << val << std::dec << '\n';
}

/// Write a 16-bit hex address in 0x%04X format.
void write_hex16(std::ostringstream& ini, const char* key, std::uint16_t val)
{
    ini << key << "=0x" << std::hex << std::uppercase
        << std::setw(4) << std::setfill('0') << val << std::dec << '\n';
}

/// Write a decimal integer key.
void write_dec(std::ostringstream& ini, const char* key, int val)
{
    ini << key << '=' << val << '\n';
}

/// Write a boolean as 1 or 0.
void write_bool(std::ostringstream& ini, const char* key, bool val)
{
    ini << key << '=' << (val ? '1' : '0') << '\n';
}

/// Write a string key.
void write_str(std::ostringstream& ini, const char* key, const std::string& val)
{
    ini << key << '=' << val << '\n';
}

} // anonymous namespace

std::string generate_ini_string(const Config& config)
{
    std::ostringstream ini;
    const auto& g = config.local_rssp1_params;
    int conn_count = static_cast<int>(config.connections.size());

    // ============================================================
    //  [RSSP1_GLOBAL]
    // ============================================================
    ini << "[RSSP1_GLOBAL]\n";
    write_dec(ini, "main_cycle",    g.main_cycle_ms);
    write_dec(ini, "connection_num", conn_count);
    write_hex16(ini, "source_addr", g.addr);
    write_dec(ini, "Max_ConnectNum", conn_count);

    // Local keys
    write_hex32(ini, "local_dataVer_A", g.keys.data_ver.a);
    write_hex32(ini, "local_dataVer_B", g.keys.data_ver.b);
    write_hex32(ini, "local_sinit_A",   g.keys.sinit.a);
    write_hex32(ini, "local_sinit_B",   g.keys.sinit.b);
    write_hex32(ini, "local_sid_A",     g.keys.sid.a);
    write_hex32(ini, "local_sid_B",     g.keys.sid.b);
    write_hex32(ini, "local_sys_chk_A", g.sys_chk.a);
    write_hex32(ini, "local_sys_chk_B", g.sys_chk.b);

    // Queue sizes
    write_dec(ini, "SFM_U2L_Q_Size_per_connection", g.queue_sizes.sfm_u2l_per_connection);
    write_dec(ini, "SFM_L2U_Q_Size_per_connection", g.queue_sizes.sfm_l2u_per_connection);
    write_dec(ini, "CFM_U2L_Q_Size_per_connection", g.queue_sizes.cfm_u2l_per_connection);
    write_dec(ini, "CFM_L2U_Q_Size_per_connection", g.queue_sizes.cfm_l2u_per_connection);
    write_dec(ini, "UsrData_All0_Size", g.usrdata_all0_size);

    // ============================================================
    //  [CON_i] -- one per connection
    // ============================================================
    for (int i = 0; i < conn_count; ++i) {
        const auto& conn = config.connections[i];

        ini << "[CON_" << i << "]\n";

        // Resolve cycle sentinels: -1 means use global main_cycle_ms.
        int fsfb_cycle = (conn.fsfb_comm_cycle_ms >= 0)
                            ? conn.fsfb_comm_cycle_ms
                            : g.main_cycle_ms;
        int node_cycle = (conn.local_node_cycle_ms >= 0)
                            ? conn.local_node_cycle_ms
                            : g.main_cycle_ms;

        write_dec(ini, "FSFB_comm_cycle",  fsfb_cycle);
        write_dec(ini, "local_node_cycle", node_cycle);
        write_hex16(ini, "dest_addr",      conn.addr);
        write_bool(ini, "enable_CRSCD_pack", conn.enable_crscd_pack);

        // Timing
        write_dec(ini, "deltaTime",      conn.timing.delta_time);
        write_dec(ini, "lifeTime",       conn.timing.life_time);
        write_dec(ini, "DelayTime",      conn.timing.delay_time);
        write_dec(ini, "torlerate_cycle", conn.timing.tolerate_cycle);

        // Connection misc
        write_dec(ini, "num_data_ver", conn.num_data_ver);
        write_bool(ini, "Is_Fix_Node", conn.is_fix_node);

        // Remote keys
        write_hex32(ini, "remote_dataVer_A", conn.keys.data_ver.a);
        write_hex32(ini, "remote_dataVer_B", conn.keys.data_ver.b);
        write_hex32(ini, "remote_sinit_A",   conn.keys.sinit.a);
        write_hex32(ini, "remote_sinit_B",   conn.keys.sinit.b);
        write_hex32(ini, "remote_sid_A",     conn.keys.sid.a);
        write_hex32(ini, "remote_sid_B",     conn.keys.sid.b);

        write_bool(ini, "remote_dev_is_A", conn.remote_dev_is_A);
        write_dec(ini, "com_type", 0); // hardcoded: UDP

        write_bool(ini, "enable_UDP_chn_FSFB", conn.enable_per_channel_fsfb);
        write_dec(ini, "Chn_apply_FSFB_ID",     conn.chn_apply_fsfb_id);
        write_dec(ini, "con_L2U_Q_size",        conn.l2u_queue_size);

        int ch_count = static_cast<int>(conn.rssp1_channels.size());
        write_dec(ini, "UDP_channel_num", ch_count);

        // Per-channel entries (local_ip_N, local_port_N, remote_ip_N, remote_port_N)
        for (int ch = 0; ch < ch_count; ++ch) {
            const auto& uc = conn.rssp1_channels[ch];
            write_str(ini, ("local_ip_"  + std::to_string(ch)).c_str(), uc.local_ip);
            write_dec(ini, ("local_port_" + std::to_string(ch)).c_str(), uc.local_port);
            write_str(ini, ("remote_ip_" + std::to_string(ch)).c_str(), uc.remote_ip);
            write_dec(ini, ("remote_port_" + std::to_string(ch)).c_str(), uc.remote_port);
        }

        // Per-connection UDP queue sizes (from first channel, or defaults)
        int recv_mq = conn.rssp1_channels.empty() ? 5 : conn.rssp1_channels[0].recv_queue_size;
        int send_mq = conn.rssp1_channels.empty() ? 5 : conn.rssp1_channels[0].send_queue_size;
        write_dec(ini, "UDP_recv_MQ_size", recv_mq);
        write_dec(ini, "UDP_send_MQ_size", send_mq);
    }

    return ini.str();
}

} // namespace forwarder
