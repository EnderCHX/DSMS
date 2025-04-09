//#include "message_hub.h"
//#pragma comment(lib, "Ws2_32.lib")
//
//
//auto main() -> int {
//    using std::cout;
//    using std::endl;
//    using std::string;
//
//    WSADATA wsaData;
//    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); // 必须调用，否则 Socket API 不可用
//    if (result != 0) {
//        cout << "WSAStartup failed with error: " << WSAGetLastError() << endl;
//        return WSAGetLastError();
//    }
//
//    message_hub::Server serverSocket("0.0.0.0", 8080);
//
//
//    message_hub::hub hub(&serverSocket);
//
//    hub.run();
//
//
//}
//
//#include <winsock2.h>
//#include <iostream>
//
//int main() {
//    WSADATA wsa;
//    WSAStartup(MAKEWORD(2,2), &wsa);
//
//    SOCKET Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    sockaddr_in addr{};
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr = INADDR_ANY;
//    addr.sin_port = htons(8080);
//    bind(Server, (sockaddr*)&addr, sizeof(addr));
//    listen(Server, SOMAXCONN);
//
//    fd_set master;
//    FD_ZERO(&master);
//    FD_SET(Server, &master);
//
//    while (true) {
//        fd_set copy = master;
//        timeval timeout{5, 0}; // 5秒超时
//
//        int count = select(0, &copy, NULL, NULL, &timeout);
//        if (count == 0) continue; // 超时
//        if (count == SOCKET_ERROR) {
//            std::cerr << "select error: " << WSAGetLastError() << "\n";
//            break;
//        }
//        for (u_int i = 0; i < master.fd_count; ++i) {
//            SOCKET sock = master.fd_array[i];
//            if (FD_ISSET(sock, &copy)) {
//                if (sock == Server) { // 新连接
//                    sockaddr_in clientAddr{};
//                    int addrLen = sizeof(clientAddr);
//                    SOCKET Client = accept(Server, (sockaddr*)&clientAddr, &addrLen);
//                    FD_SET(Client, &master);
//                    std::cout << "New Client connected\n";
//                } else { // 客户端数据可读
//                    char buf[1024];
//                    int bytes = recv(sock, buf, sizeof(buf), 0);
//                    if (bytes <= 0) { // 连接关闭或错误
//                        closesocket(sock);
//                        FD_CLR(sock, &master);
//                    } else {
//                        // 处理接收到的数据
//                        send(sock, buf, bytes, 0); // 回显
//                    }
//                }
//            }
//        }
//    }
//
//    closesocket(Server);
//    WSACleanup();
//    return 0;
//}

//#include <iostream>
//#include <memory>
//#include <boost/asio.hpp>
//
//using boost::asio::ip::tcp;
//using namespace boost::system;
//
//class Session : public std::enable_shared_from_this<Session> {
//public:
//    Session(tcp::socket socket) : socket_(std::move(socket)) {}
//
//    void start() {
//        do_read();
//    }
//
//private:
//    void do_read() {
//        auto self(shared_from_this());
//        socket_.async_read_some(
//                boost::asio::buffer(data_, max_length),
//                [this, self](error_code ec, size_t length) {
//                    if (!ec) {
//                        do_write(length);
//                    }
//                });
//    }
//
//    void do_write(size_t length) {
//        auto self(shared_from_this());
//        boost::asio::async_write(
//                socket_,
//                boost::asio::buffer(data_, length),
//                [this, self](error_code ec, size_t /*length*/) {
//                    if (!ec) {
//                        do_read();
//                    }
//                });
//    }
//
//    tcp::socket socket_;
//    enum { max_length = 1024 };
//    char data_[max_length];
//};
//
//class Server {
//public:
//    Server(boost::asio::io_context& io_context, short port)
//            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
//        do_accept();
//    }
//
//private:
//    void do_accept() {
//        acceptor_.async_accept(
//                [this](error_code ec, tcp::socket socket) {
//                    if (!ec) {
//                        std::make_shared<Session>(std::move(socket))->start();
//                    }
//                    do_accept();  // 继续接受新连接
//                });
//    }
//
//    tcp::acceptor acceptor_;
//};
//
//int main(int argc, char* argv[]) {
//    try {
////        if (argc != 2) {
////            std::cerr << "Usage: " << argv[0] << " <port>\n";
////            return 1;
////        }
//
//        boost::asio::io_context io_context;
//        Server s(io_context, 8080);
//        io_context.run();  // 启动事件循环
//    } catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//    return 0;
//}
//
#include "message_hub.h"

auto main() -> int {
    message_hub::Hub hub("0.0.0.0", 8080);
    return 0;
}