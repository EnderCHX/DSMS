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
#include "log.h"

namespace CHX {
    namespace MessageHub {
            using json = nlohmann::json;

    class Client;
    class Server;
    class Hub;

    extern json heartbeat_packet;
    extern CHX::Log logger;

    extern std::queue<std::string> message_send;
    extern std::mutex message_send_lock;
    extern std::queue<std::pair<std::string, Client*>> message_recv;
    extern std::mutex message_recv_lock;

    extern std::unordered_set<Client*> client_set;
    extern std::mutex client_set_lock;

    extern std::queue<Client*> disconnected_client_queue;
    extern std::mutex disconnected_client_queue_lock;

    extern std::unordered_map<Client*, std::vector<std::string>> event_publisher_map;
    extern std::mutex event_publisher_map_lock;

    extern std::unordered_map<std::string, std::unordered_set<Client*>> event_map;
    extern std::mutex event_map_lock;

    extern std::mutex cout_lock;

    class Client {
    public:
        Client(boost::asio::ip::tcp::socket socket, boost::asio::io_context* ioc) : sock(std::move(socket)), timer(*ioc, heartbeat_interval) {
            this->ioc = ioc;
            streambuf.prepare(128);
        }

        auto start() -> void {
            do_heartbeat();
            do_read();
        }

        auto stop() -> void {
            sock.close();
        }

        auto addr() -> std::string {
            return sock.remote_endpoint().address().to_string();
        }

        auto port() -> int {
            return sock.remote_endpoint().port();
        }

    private:
        auto do_read() ->void {
            boost::asio::async_read_until(sock, streambuf, '\n',[&](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    streambuf.commit(length);
                    std::istream is(&streambuf);
                    std::string msg;
                    is >> msg;
                    {
                        std::lock_guard<std::mutex> lock(message_recv_lock);
                        message_recv.emplace(msg, this);
                    }
                    logger.info(std::format("client {}:{} msg -> {} -> queue message_recv", addr(), port(), msg));
                    do_write(msg);
                    do_read();
                } else {
                    logger.error(ec.what());
                }
            });
        }

        auto do_write(std::string msg) ->void {
            try {
                auto index = std::find(msg.begin(), msg.end(), '\n');
                msg.replace(index, msg.end(), "");
                sock.async_write_some(boost::asio::buffer(msg), [this](boost::system::error_code ec, size_t /*length*/) {});
            } catch (...) {
                logger.error(std::format("write error -> {}:{}", addr(), port()));
            }
        }
        auto do_heartbeat() -> void {
            timer.async_wait([this](const boost::system::error_code& ec) {
                if (!ec) {
                    logger.info(std::format("heartbeat -> {}:{}", addr(), port()));
                    sock.async_write_some(boost::asio::buffer(heartbeat_packet.dump()+"\r"), [this](boost::system::error_code ec, size_t /*length*/) {
                        if (!ec) {
                            timer.expires_after(heartbeat_interval);
                            do_heartbeat();
                        } else {
                            logger.info(std::format("heartbeat error -> {}:{}", addr(), port()));
                            {
                                std::lock_guard<std::mutex> lock(disconnected_client_queue_lock);
                                disconnected_client_queue.emplace(this);
                            }
                            logger.info(std::format("add {}:{} to -> disconnected_client_queue", addr(), port()));
                        }
                    });
                } else {
                    logger.error(ec.what());
                }
            });
        }

        boost::asio::io_context* ioc;
        boost::asio::steady_timer timer;
        std::chrono::seconds heartbeat_interval = std::chrono::seconds(30);
        boost::asio::ip::tcp::socket sock;
        boost::asio::streambuf streambuf;
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
                    auto client = new CHX::MessageHub::Client(std::move(sock), &ioc);
                    client->start();

                    {
                        std::lock_guard<std::mutex> lock(client_set_lock);
                        client_set.insert(client);
                    }
                    logger.info(std::format("new client connected -> {}:{}", client->addr(), client->port()));
                    logger.info(std::format("client count: {}", client_set.size()));
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
                            msg = std::string(message_recv.front().first);
                            message_recv.pop();
                        }
                        logger.info(std::format("pop queue message_recv msg -> {}", msg));
                        // try {
                        //      json json_msg;
                        //      json_msg = json::parse(msg);
                        //      if (json_msg["type"] == "event") {
                        //          auto event_name = json_msg["event"];
                        //      }
                        //  } catch (json::parse_error &e) {
                        //      logger.error(e.what());
                        //  } catch(std::exception &e) {
                        //      logger.error(e.what());
                        //  } catch (...) {
                        //      logger.error("unknown error");
                        //  }
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
                        logger.info(std::format("remove {}:{} from clients_pool", client->addr(), client->port()));
                        client->stop();
                        delete client;
                        logger.info(std::format("client count: {}", client_set.size()));
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
}
#endif //DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H_BAK
