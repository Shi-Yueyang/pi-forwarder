#pragma once

// Generates the RSSP1 INI configuration string from a parsed Config struct.
// The output is passed to GM_RSSP1_APP_Interface_Init() with is_path=false.
// Never writes to disk.

#include <string>

namespace forwarder {

struct Config;

/// Generate the complete RSSP1 INI content from the parsed Config.
std::string generate_ini_string(const Config& config);

} // namespace forwarder
