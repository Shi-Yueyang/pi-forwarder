#include "rssp1_adapter.hpp"
#include "log.hpp"

// RSSP1 C headers — all inside extern "C" blocks.
// These headers already have their own __cplusplus / extern "C" guards.
extern "C" {
#include "GM_RSSP1_Define.h"
#include "GM_RSSP1_APP_Interface.h"
#include "GM_RSSP1_CFM_Interface.h"
#include "GM_RSSP1_SFM_Interface.h"
#include "GM_RSSP1_Utils.h"
#include "common/GM_RSSP1_VSN.h"
#include "common/GM_RSSP1_Utils_Other.h"
}

#include <cstring>
#include <stdexcept>
#include <utility>

namespace forwarder {

// ============================================================================
// C-callable callback stubs (must have C linkage)
// ============================================================================

namespace {

// Bridge: C log-usr callback -> C++ LOG_WARN
void forward_log_usr(unsigned long errNo, int, int, int, int, int, int)
{
    LOG_WARN("RSSP1 internal: err=0x" +
             std::to_string(errNo));  // hex would be better, but to_string works
}

} // anonymous namespace

extern "C" {

/// ABAS callback: returns A-machine, Active, Peer=Active (0x3333).
/// The C stack calls this to determine local active/standby role.
static GM_RSSP1_UINT16 rssp1_get_abas(void)
{
    return 0x3333;  // ABAS_Type_A_AA
}

/// VSN callback: delegates to the built-in VSN implementation.
static void rssp1_vsn_get(GM_RSSP1_UINT32* pVSN0,
                           GM_RSSP1_UINT32* pVSN1,
                           GM_RSSP1_UINT32* pVSN2)
{
    GM_RSSP1_VSN_Get(pVSN0, pVSN1, pVSN2);
}

/// Log-usr callback: receives internal error reports from the C stack.
static void rssp1_log_usr(unsigned long errNo,
                           int arg1, int arg2, int arg3,
                           int arg4, int arg5, int arg6)
{
    forward_log_usr(errNo, arg1, arg2, arg3, arg4, arg5, arg6);
}

} // extern "C"

// ============================================================================
// Rssp1Adapter
// ============================================================================

Rssp1Adapter::Rssp1Adapter() = default;
Rssp1Adapter::~Rssp1Adapter() = default;

// ---- Initialization ----

bool Rssp1Adapter::init(const std::string& ini_content)
{
    LOG_INFO("Initializing RSSP1 stack (v" +
             std::string(GM_RSSP1_TABLE_Ver) + ")");

    // 1. Initialize the built-in VSN (timestamp generator)
    if (GM_RSSP1_VSN_Init() == GM_RSSP1_FALSE) {
        LOG_ERROR("RSSP1 VSN initialization failed");
        return false;
    }

    // 2. Register the log-usr callback so the C stack reports errors
    //    through our structured logger instead of calling printf directly.
    GM_RSSP1_Log_Usr_Callback_Init(rssp1_log_usr);

    // 3. Copy the INI string to a mutable buffer.
    //    The C API takes char* (not const char*) because when is_path=false,
    //    it parses the string in-place with strtok-like functions.
    std::vector<char> ini_buf(ini_content.begin(), ini_content.end());
    ini_buf.push_back('\0');

    // 4. Initialize the stack with the INI content in memory
    GM_RSSP1_BOOL result = GM_RSSP1_APP_Interface_Init(
        rssp1_get_abas,          // ABAS (active/standby) callback
        ini_buf.data(),          // INI content (mutable char*)
        GM_RSSP1_FALSE,          // is_path=false -> memory buffer
        rssp1_vsn_get            // VSN callback
    );

    if (result == GM_RSSP1_FALSE) {
        LOG_ERROR("RSSP1 stack initialization failed");
        return false;
    }

    initialized_ = true;
    LOG_INFO("RSSP1 stack initialized successfully");
    return true;
}

// ---- Data input ----

bool Rssp1Adapter::rcv_com_interface(const std::vector<std::uint8_t>& frame,
                                      std::uint32_t src_ip,
                                      std::uint32_t src_port,
                                      std::uint8_t mode)
{
    if (!initialized_) return false;

    return GM_RSSP1_RCV_com_Interface(
               const_cast<GM_RSSP1_UINT8*>(frame.data()),
               static_cast<GM_RSSP1_INT16>(frame.size()),
               src_ip,
               src_port,
               0,     // index (ignored when mode=1)
               0,     // chn_index (ignored when mode=1)
               mode
           ) == GM_RSSP1_TRUE;
}

// ---- Cycle processing ----

void Rssp1Adapter::receive_pass()
{
    if (!initialized_) return;
    GM_RSSP1_APP_Interface_RxPrc();
}

void Rssp1Adapter::send_pass()
{
    if (!initialized_) return;
    GM_RSSP1_APP_Interface_TxPrc();
}

int Rssp1Adapter::drain_received(std::vector<ReceivedPayload>& out)
{
    if (!initialized_) return -1;

    // Buffer large enough for max user data + RSSP1 internal header
    GM_RSSP1_UINT8  buf[GM_RSSP1_MAX_USER_DAT_LEN + 64];
    GM_RSSP1_UINT32 src_addr = 0;
    GM_RSSP1_UINT32 data_len = 0;
    GM_RSSP1_INT32  remaining = 0;

    GM_RSSP1_INT8 rc = GM_RSSP1_APP_Interface_Rcv_App_Dat(
        buf, &src_addr, &data_len, &remaining);

    if (rc <= 0) return rc;  // -1 = no more, 0 = non-app message

    // The first 6 bytes of the returned data are the RSSP1 internal header.
    // The actual application payload starts at buf + 6 with length data_len - 6.
    if (data_len > 6) {
        ReceivedPayload p;
        p.source_addr = src_addr;
        p.data.assign(buf + 6, buf + data_len);
        out.push_back(std::move(p));
    }
    return 1;
}

bool Rssp1Adapter::send_app_data(const std::vector<std::uint8_t>& data,
                                  std::uint16_t dest_addr)
{
    if (!initialized_) return false;

    // GM_RSSP1_SNDPROC_ON_USRDATA_MEM is defined: the stack writes a
    // 6-byte header in-place BEFORE the data pointer we pass.
    // Allocate a buffer with 6 bytes of headroom.
    std::vector<GM_RSSP1_UINT8> buf(6, 0);
    buf.insert(buf.end(), data.begin(), data.end());

    return GM_RSSP1_APP_Interface_Send_App_Dat(
               buf.data() + 6,  // user data starts after 6-byte headroom
               static_cast<GM_RSSP1_INT16>(data.size()),
               dest_addr
           ) == GM_RSSP1_TRUE;
}

bool Rssp1Adapter::drain_to_send(SendFrame& out)
{
    if (!initialized_) return false;

    GM_RSSP1_UINT8  buf[GM_RSSP1_MAX_USER_DAT_LEN + 256];
    GM_RSSP1_UINT16 len = 0;
    GM_RSSP1_UINT32 ip = 0;
    GM_RSSP1_UINT32 port = 0;
    GM_RSSP1_UINT16 index = 0;
    GM_RSSP1_UINT8  chn_index = 0;

    if (GM_RSSP1_SND_com_Interface(buf, &len, &ip, &port, &index, &chn_index)
        == GM_RSSP1_TRUE)
    {
        out.data.assign(buf, buf + len);
        out.ip = ip;               // network byte order
        out.port = port;
        out.conn_index = index;
        out.chn_index = chn_index;
        return true;
    }
    return false;
}

// ---- Status / control ----

void Rssp1Adapter::set_log_level(std::uint16_t level)
{
    GM_RSSP1_Set_LogLevel(level);
}

void Rssp1Adapter::vsn_update()
{
    if (!initialized_) return;
    GM_RSSP1_VSN_Update();
}

} // namespace forwarder
