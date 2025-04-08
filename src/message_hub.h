//
// Created by c on 2025/4/8.
//

#ifndef DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H
#define DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <winsock2.h>

namespace message_hub {

    using std::unordered_map;
    using std::unordered_set;
    using std::vector;
    using std::string;

    class client {

    };

    class event {

    };

    class server {
    public:
        server(string ip, int port) {
            this->server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            this->server_addr.sin_family = AF_INET;
            this->server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
            this->server_addr.sin_port = htons(port);
            bind(this->server_socket, (sockaddr*)&this->server_addr, sizeof(this->server_addr));
        }
        ~server() {
            closesocket(this->server_socket);
        }

        auto listen() -> void {
            ::listen(this->server_socket, SOMAXCONN);
        }

        auto accept() -> client {
            sockaddr_in client_addr;
            int clientAddrLen = sizeof(client_addr);
            SOCKET client_socket = ::accept(this->server_socket, (sockaddr*)&client_socket, &clientAddrLen);
            client client(client_socket);
            return client;
        }
    private:
        SOCKET server_socket;
        sockaddr_in server_addr;
    };

    using std::vector;
    class hub {
        hub(server server) {
            server = server();
        }

    private:
        server server;
        unordered_set<client> *clients;
        unordered_map<event, vector<client*>> event_clients;
    };


}


#endif //DISTRIBUTED_SIMULATION_MANAGEMENT_SYSTEM_MESSAGE_HUB_H
