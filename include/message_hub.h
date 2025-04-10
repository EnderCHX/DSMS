//
// Created by c on 2025/4/8.
//

#ifndef DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK
#define DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/system.hpp>
#include <nlohmann/json.hpp>

namespace message_hub {

    using json = nlohmann::json;

    class Client;
    class Server;
    class Hub;

    extern json heartbeat_packet;

    extern std::queue<std::string> message_send;
    extern std::mutex message_send_lock;
    extern std::queue<std::string> message_recv;
    extern std::mutex message_recv_lock;

    extern std::unordered_set<Client*> client_set;
    extern std::mutex client_set_lock;

    extern std::queue<Client*> disconnected_client_queue;
    extern std::mutex disconnected_client_queue_lock;

    extern std::unordered_map<std::string, Client*> event_map;
    extern std::mutex event_map_lock;

    class Client {
    public:
        Client(boost::asio::ip::tcp::socket socket, boost::asio::io_context* ioc) : sock(std::move(socket)), buffer(1024), msg(""), timer(*ioc, heartbeat_interval) {
            this->ioc = ioc;
            msg.reserve(1024);
        }

        auto start() -> void {
            do_heartbeat();
            do_read();
        }

        auto stop() -> void {
            sock.close();
        }

    private:
        auto do_read() ->void {
            sock.async_read_some(boost::asio::buffer(buffer),[this](boost::system::error_code ec, size_t length){
                if (!ec) {
                    if (std::find(buffer.begin(), buffer.end(), '\r') != buffer.end()) {
                        auto index = std::find(buffer.begin(), buffer.end(), '\r');
                        if (*(index + 1) == '\n') {
                            msg += std::string(buffer.begin(), index+1);
                            buffer.erase(buffer.begin(), index + 1);
                            {
                                std::lock_guard<std::mutex> lock(message_recv_lock);
                                message_recv.emplace(msg);
                            }
                            std::cout << msg << " l=" << message_recv.size() << std::endl;
                            do_write();
                            msg.clear();
                        }
                    } else {
                        msg += std::string(buffer.begin(), buffer.end());
                    }
                    do_read();
                }
            });
        }

        auto do_write() ->void {
            boost::asio::async_write(
                    sock,
                    boost::asio::buffer(msg),
                    [this](boost::system::error_code ec, size_t /*length*/) {
                        if (!ec) {
                            do_read();
                        }
                    });
        }
        auto do_heartbeat() -> void {
            timer.async_wait([this](const boost::system::error_code& ec) {
                if (!ec) {
                    std::cout << "heartbeat" << std::endl;
                    sock.async_write_some(boost::asio::buffer(heartbeat_packet.dump()+"\r\n"), [this](boost::system::error_code ec, size_t /*length*/) {
                        if (!ec) {
                            timer.expires_after(heartbeat_interval);
                            do_heartbeat();
                        } else {
                            std::cout << "heartbeat error1" << std::endl;
                            {
                                std::lock_guard<std::mutex> lock(disconnected_client_queue_lock);
                                disconnected_client_queue.emplace(this);
                            }
                        }
                    });
                } else {
                    std::cout << "heartbeat error" << std::endl;
                }
            });
        }

        boost::asio::io_context* ioc;
        boost::asio::steady_timer timer;
        std::chrono::seconds heartbeat_interval = std::chrono::seconds(1);
        boost::asio::ip::tcp::socket sock;
        std::vector<char> buffer;
        std::string msg;
    };

    class Server {
    public:
        Server(const std::string &ip, int port) : ac(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {
        }

        auto run() -> void {
            do_accept();
            ioc.run();
        }

        auto do_accept() -> void {
            ac.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket sock){
                if (!ec) {
                    auto client = new message_hub::Client(std::move(sock), &ioc);
                    client->start();

                    {
                        std::lock_guard<std::mutex> lock(client_set_lock);
                        client_set.insert(client);
                    }

                    std::cout << "new client connected" << std::endl;
                    std::cout << "client count: " << client_set.size() << std::endl;
                    do_accept();
                }
            });
        }

    private:
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::acceptor ac;
    };

    class Hub {
    public:
        Hub(const std::string &ip, int port) : server(ip, port) {
            server_thread = std::thread([&](){
                server.run();
            });
            send_thread = std::thread([&](){
                while (true) {
                    while (!message_send.empty()) {
                        std::lock_guard<std::mutex> lock(message_send_lock);
                        json msg = json::parse(message_send.front());
                        if (msg["option"] == "publish") {
                            if (msg["data"]["type"] == "register") {
                                msg["data"]["event"];
                            }
                        } else if (msg["option"] == "subscribe") {

                        }
                        message_send.pop();
                    }
                }
            });
            recv_thread = std::thread([&](){
                while (true) {
                    while (!message_recv.empty()) {
                        std::string msg;
                        {
                            std::lock_guard<std::mutex> lock(message_recv_lock);
                            msg = message_recv.front();
                            message_recv.pop();
                        }
                        std::cout << "recv: " << msg << std::endl;
                        
                        // std::cout << "recv: " << msg << std::endl;
                        // json json_msg;
                        // try {
                        //     json_msg = json::parse(msg);
                        // } catch (json::parse_error &e) {
                        //     std::cout << "parse error: " << e.what() << std::endl;
                        //     continue;
                        // }
                        // if (json_msg["type"] == "event") {
                        //     auto event_name = json_msg["event"];
                        // }
                    }
                }
            });
            heartbeat_thread = std::thread([&](){
                while (true) {
                    while (!disconnected_client_queue.empty()) {
                        std::lock_guard<std::mutex> lock1(disconnected_client_queue_lock);
                        auto client = disconnected_client_queue.front();
                        disconnected_client_queue.pop();
                        {
                            std::lock_guard<std::mutex> lock(client_set_lock);
                            client_set.erase(client);
                        }
                        client->stop();
                        delete client;
                    }
                }
            });
            server_thread.join();
            heartbeat_thread.join();
            recv_thread.join();
            send_thread.join();
        }
    private:


        Server server;
        std::thread server_thread;
        std::thread send_thread;
        std::thread recv_thread;
        std::thread heartbeat_thread;
    };
}
#endif //DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK
