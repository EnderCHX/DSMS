#include "config.h"
#include "message_hub.h"


auto main() -> int {
    CHX::Config config("config.json");
    std::string addr = "0.0.0.0";
    int port = 8080;
    CHX::Log logger;
    logger.Info(config.GetConfig().dump());
    logger.Info(std::format("Server Start at {}:{}", addr, port));
    CHX::MessageHub::Hub hub(addr, port);
    return 0;
}