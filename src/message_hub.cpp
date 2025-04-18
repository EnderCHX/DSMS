//
// Created by c on 2025/4/8.
//

#include "message_hub.h"

namespace CHX {
    namespace MessageHub {
        std::queue<json> message_send = std::queue<json>();
        std::mutex message_send_lock;
        std::queue<std::pair<std::string, Client*>> message_recv = std::queue<std::pair<std::string, Client*>>();
        std::mutex message_recv_lock;

        std::unordered_set<Client*> client_set = std::unordered_set<Client*>();
        std::mutex client_set_lock;

        std::queue<Client*> disconnected_client_queue = std::queue<Client*>();
        std::mutex disconnected_client_queue_lock;

        // std::unordered_map<Client*, std::vector<std::string>> event_publisher_map = std::unordered_map<Client*, std::vector<std::string>>();
        // std::mutex event_publisher_map_lock;

        std::unordered_map<std::string, std::unordered_set<Client*>> event_map = std::unordered_map<std::string, std::unordered_set<Client*>>();
        std::mutex event_map_lock;

        std::mutex cout_lock;

        json heartbeat_packet = json::parse(R"({"option":"ping"})");
        CHX::Log logger = CHX::Log();
    }

    namespace MessageHub {
        //Client
        Client::Client(boost::asio::ip::tcp::socket socket, boost::asio::io_context* ioc): sock(std::move(socket)), timer(*ioc, heartbeat_interval) {
            this->ioc = ioc;
            streambuf.prepare(128);
        }
        auto Client::start() -> void {
            do_heartbeat();
            do_read();
        }
        auto Client::stop() -> void {
            sock.close();
        }
        auto Client::addr() const -> std::string {
            return sock.remote_endpoint().address().to_string();
        }
        auto Client::port() const -> int {
            return sock.remote_endpoint().port();
        }
        auto Client::do_read() -> void {
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
                    logger.Info(std::format("client {}:{} msg -> {} -> queue message_recv", addr(), port(), msg));
                    do_write(msg);
                    do_read();
                } else {
                    logger.Error(ec.what());
                }
            });
        }
        auto Client::do_write(std::string msg) -> void {
            try {
                sock.async_write_some(boost::asio::buffer(msg), [this](boost::system::error_code ec, size_t /*length*/) {});
            } catch (...) {
                logger.Error(std::format("write error -> {}:{}", addr(), port()));
            }
        }
        auto Client::do_heartbeat() -> void {
            timer.async_wait([this](const boost::system::error_code& ec) {
                if (!ec) {
                    logger.Info(std::format("heartbeat -> {}:{}", addr(), port()));
                    sock.async_write_some(boost::asio::buffer(heartbeat_packet.dump()+"\r"), [this](boost::system::error_code ec, size_t /*length*/) {
                        if (!ec) {
                            timer.expires_after(heartbeat_interval);
                            do_heartbeat();
                        } else {
                            logger.Info(std::format("heartbeat error -> {}:{}", addr(), port()));
                            {
                                std::lock_guard<std::mutex> lock(disconnected_client_queue_lock);
                                disconnected_client_queue.emplace(this);
                            }
                            logger.Info(std::format("add {}:{} to -> disconnected_client_queue", addr(), port()));
                        }
                    });
                } else {
                    logger.Error(ec.what());
                }
            });
        }
    }

    namespace MessageHub {
        //Server
        Server::Server(const std::string &ip, int port) : ac(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(ip), port)) {
        }
        auto Server::run() -> void {
            do_accept();
            ioc.run();
        }
        auto Server::do_accept() -> void {
            ac.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket sock){
                if (!ec) {
                    auto client = Client(std::move(sock), &ioc);
                    client.start();

                    {
                        std::lock_guard lock(client_set_lock);
                        client_set.insert(&client);
                    }
                    logger.Info(std::format("new client connected -> {}:{}", client.addr(), client.port()));
                    logger.Info(std::format("client count: {}", client_set.size()));
                    do_accept();
                }
            });
        }
    }

    namespace MessageHub {
        //hub
        Hub::Hub(const std::string &ip, int port) : server(ip, port) {
            server_thread = std::thread([&](){
                server.run();
            });
            send_thread = std::thread([&](){
                while (true) {
                    while (!message_send.empty()) {
                        std::lock_guard lock(message_send_lock);
                        auto msg = message_send.front();
                        if (auto event = msg["data"]["event"].dump(); event_map.contains(event)) {
                            for (auto clients = event_map[event]; auto client : clients) {
                                client->do_write(msg.dump()+"\n");
                            }
                        }
                        message_send.pop();
                    }
                }
            });
            recv_thread = std::thread([&](){
                while (true) {
                    while (!message_recv.empty()) {
                        std::pair<std::string, Client*> msg;
                        {
                            std::lock_guard lock(message_recv_lock);
                            msg = message_recv.front();
                            message_recv.pop();
                        }
                        logger.Info(std::format("pop queue message_recv msg -> {}", msg.first));
                        try {
                            json json_msg = json::parse(std::string(msg.first));
                            handleMsg(json_msg, msg.second);
                        } catch (json::parse_error &e) {
                            logger.Error(e.what());
                        } catch(std::exception &e) {
                            logger.Error(e.what());
                        } catch (...) {
                            logger.Error("unknown error");
                        }
                    }
                }
            });
            heartbeat_thread = std::thread([&](){
                while (true) {
                    while (!disconnected_client_queue.empty()) {
                        std::lock_guard lock1(disconnected_client_queue_lock);
                        auto client = disconnected_client_queue.front();
                        disconnected_client_queue.pop();
                        {
                            std::lock_guard lock(client_set_lock);
                            client_set.erase(client);
                        }
                        logger.Info(std::format("remove {}:{} from clients_pool", client->addr(), client->port()));
                        client->stop();
                        delete client;
                        logger.Info(std::format("client count: {}", client_set.size()));
                    }
                }
            });
            server_thread.join();
            heartbeat_thread.join();
            recv_thread.join();
            send_thread.join();
        }

        auto Hub::handleClientClose(Client *client) -> void {
            client->stop();
        }


        auto Hub::handleMsg(json msg, Client *client) -> void {
            if (!msg.contains("option")) {return;}
            if (msg["option"] == "publish") { handlePublish(msg, client);}
            if (msg["option"] == "subscribe") { handleSubscribe(msg["data"], client);}
        }
        auto Hub::handlePublish(json msg, Client *client) -> void {
            if (!msg["data"].contains("event")) {
                return;
            }
            {
                std::lock_guard lock(message_send_lock);
                message_send.emplace(msg);
            }
        }
        auto Hub::handleSubscribe(json data, Client *client) -> void {
            if (!event_map.contains(data["event"])) {
                return;
            }
            {
                std::lock_guard lock(event_map_lock);
                event_map[data["event"]].emplace(client);
            }
        }
    }
}