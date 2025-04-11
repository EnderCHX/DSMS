//
// Created by c on 2025/4/8.
//

#include "message_hub.h"

namespace message_hub {
    std::queue<std::string> message_send = std::queue<std::string>();
    std::mutex message_send_lock;
    std::queue<std::pair<std::string, Client*>> message_recv = std::queue<std::pair<std::string, Client*>>();
    std::mutex message_recv_lock;

    std::unordered_set<Client*> client_set = std::unordered_set<Client*>();
    std::mutex client_set_lock;

    std::queue<Client*> disconnected_client_queue = std::queue<Client*>();
    std::mutex disconnected_client_queue_lock;

    std::unordered_map<Client*, std::vector<std::string>> event_publisher_map = std::unordered_map<Client*, std::vector<std::string>>();
    std::mutex event_publisher_map_lock;

    std::unordered_map<std::string, std::unordered_set<Client*>> event_map = std::unordered_map<std::string, std::unordered_set<Client*>>();
    std::mutex event_map_lock;

    std::mutex cout_lock;

    json heartbeat_packet = json::parse(R"({"option":"ping"})");
    chx_log::Log logger = chx_log::Log();
}


