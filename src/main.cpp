#include "message_hub.h"


auto main() -> int {
    std::string addr = "0.0.0.0";
    int port = 8080;
    CHX::Log logger;
    logger.info(std::format("Server Start at {}:{}", addr, port));
    CHX::MessageHub::Hub hub(addr, port);
    return 0;
}