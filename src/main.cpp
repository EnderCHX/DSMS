#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")


auto main() -> int {
    using std::cout;
    using std::endl;
    using std::string;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData); // 必须调用，否则 Socket API 不可用
    if (result != 0) {
        cout << "WSAStartup failed with error: " << result << endl;
        return 1;
    }

    // 1. 创建 Socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 2. 绑定地址和端口
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0"); // 监听所有网卡
    serverAddr.sin_port = htons(8080);       // 端口号
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 3. 开始监听
    listen(serverSocket, SOMAXCONN); // SOMAXCONN 为最大队列长度

    // 4. 接受客户端连接
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);

    // 5. 收发数据
    while (true) {
        char buffer[1024];
        int recvLen = recv(clientSocket, buffer, sizeof(buffer), 0);
        string str(buffer, recvLen);
        cout << "Received: " << str <<  endl;
        send(clientSocket, str.c_str(), recvLen, 0);
    }


    // 6. 关闭 Socket
    closesocket(clientSocket);
    closesocket(serverSocket);
}