// Entry point for the UDP <-> RSSP1 forwarder.

#include "forwarder/forwarder.hpp"
#include "forwarder/config.hpp"
#include "forwarder/log.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[])
{
    // ---- CLI ----
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Usage: forwarder [config_path]\n"
                     "  config_path   Path to JSON config file (default: config/forwarder.json)\n";
        return 0;
    }

    std::string config_path = "config/forwarder.json";
    if (argc > 1) {
        config_path = argv[1];
    }

    // ---- Parse config, init logging, run ----
    try {
        auto config = forwarder::parse_config_file(config_path);
        forwarder::set_log_level(config.log_level);

        LOG_INFO("Using config file: " + config_path);

        forwarder::Forwarder app(config);
        return app.run();

    } catch (const std::exception& e) {
        std::cerr << "FATAL: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
