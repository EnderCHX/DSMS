#include "message_hub.h"

auto main() -> int {
    message_hub::Hub hub("0.0.0.0", 8080);
    return 0;
}