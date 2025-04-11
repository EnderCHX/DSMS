#include "message_hub.h"


auto main() -> int {
    std::string addr = "0.0.0.0";
    int port = 8080;
    chx_log::Log logger;
    logger.info(std::format("Server Start at {}:{}", addr, port));
    message_hub::Hub hub(addr, port);
    return 0;
}