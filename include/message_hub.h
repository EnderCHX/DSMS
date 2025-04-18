//
// Created by c on 2025/4/8.
//

#ifndef DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK
#define DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK


#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <utility>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "log.h"

namespace CHX {
    namespace MessageHub {

    using json = nlohmann::json;

    class Client;
    class Server;
    class Hub;

    extern json heartbeat_packet;
    extern CHX::Log logger;

    extern std::queue<json> message_send;                       //message send queue
    extern std::mutex message_send_lock;                               //message send queue lock
    extern std::queue<std::pair<std::string, Client*>> message_recv;   //message receive queue
    extern std::mutex message_recv_lock;                               //message receive queue lock

    extern std::unordered_set<Client*> client_set;                     //connected clients set
    extern std::mutex client_set_lock;                                 //connected clients set lock

    extern std::queue<Client*> disconnected_client_queue;              //clients need to be deleted queue
    extern std::mutex disconnected_client_queue_lock;

    // extern std::unordered_map<Client*, std::vector<std::string>> event_publisher_map; //publisher and it's event list
    // extern std::mutex event_publisher_map_lock;

    extern std::unordered_map<std::string, std::unordered_set<Client*>> event_map;
    extern std::mutex event_map_lock;

    extern std::mutex cout_lock;

    class Client {
    public:
        Client(boost::asio::ip::tcp::socket socket, boost::asio::io_context* ioc);
        auto start() -> void;
        auto stop() -> void;
        auto addr() const -> std::string;
        auto port() const -> int;
        auto do_write(std::string msg) ->void;
    private:
        auto do_read() ->void;
        auto do_heartbeat() -> void;
        boost::asio::io_context* ioc;
        boost::asio::steady_timer timer;
        std::chrono::seconds heartbeat_interval = std::chrono::seconds(30);
        boost::asio::ip::tcp::socket sock;
        boost::asio::streambuf streambuf;
    };

    class Server {
    public:
        Server(const std::string &ip, int port);
        auto run() -> void;
        auto do_accept() -> void;
    private:
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::acceptor ac;
    };

    class Hub {
    public:
        Hub(const std::string &ip, int port);
    private:
        Server server;
        std::thread server_thread;
        std::thread send_thread;
        std::thread recv_thread;
        std::thread heartbeat_thread;
        // ThreadPool th_pool;

        static auto handleClientClose(Client* client) -> void;
        static auto handleMsg(json msg, Client* client) -> void;

        static auto handlePublish(json data, Client* client) -> void;
        static auto handleSubscribe(json data, Client* client) -> void;
     };
    }
}
#endif //DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK
