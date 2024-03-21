#ifndef SCTP_POC_COMMON_H
#define SCTP_POC_COMMON_H

#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <unistd.h>

constexpr static uint16_t PORT = 4141;
constexpr static uint32_t ADDR = 2130706433;
/***
A one-to-many style interface with 1 to MANY relationship between
socket and associations where the outbound association setup is
implicit. The syntax of a one-to-many style socket() call is

sd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);

A typical server in this style uses the following socket calls in
sequence to prepare an endpoint for servicing requests.

1. socket()
2. bind()
3. listen()
4. recvmsg()
5. sendmsg()
6. close()

A typical client uses the following calls in sequence to setup an
association with a server to request services.

1. socket()
2. sendmsg()
3. recvmsg()
4. close()
 */

class Socket {
public:
    explicit Socket(int fd) : _fd{fd} {
        std::cout << "FILE DESCRIPTOR = " << _fd << '\n';
    }
    //template <typename O, typename S>
    //void set_socket_option();

    ~Socket() {
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "~Socket()" << '\n';
        _receiverThread.join();
    }

    int send(unsigned char* buf, size_t msgLen) const {
        return -1;
    };

    const int get_socket_descriptor() const {
        return _fd;
    }

    /**
     * Starts the _receiverThread
     * which invokes _on_open()
     */
    void open() {
        _receiverThread = std::thread{&Socket::_on_open, this};
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "open()" << '\n';
    }

private:
    /**
     * endless loop
     * invokes _data_recv_callback()
     */
    void _on_open() {
        char inputBuffer[4096];
        size_t inputBufferSize{4096};
        while(true) {
            std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "_on_open()"  << '\n';
            sockaddr_in addr;
            (void)memset(&addr, 0, sizeof(sockaddr_in));
            sctp_sndrcvinfo sri;
            (void)memset(&sri, 0, sizeof(sctp_sndrcvinfo));
            socklen_t len = sizeof(addr);
            int msgLen{0};
            int flag{0};
            std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "waiting on sctp_recvmsg()"  << '\n';
            msgLen =  sctp_recvmsg(_fd, inputBuffer, inputBufferSize, (sockaddr*)&addr, &len, &sri, &flag);
            if (-1 == msgLen) {
                std::cout << "SCTP read failed, errno : " << errno << ", error : " << strerror(errno) << '\n';
            } else {
                uint32_t ipAddr = addr.sin_addr.s_addr;
                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "Read bytes : " << msgLen << ", input buffer size : " << inputBufferSize << ", from address : " << ipAddr << '\n';
                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "PPID : " << sri.sinfo_ppid << '\n'; //client code
                if (sri.sinfo_assoc_id == 0) {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "Assoc id = 0" << '\n';
                } else {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Assoc id => " << sri.sinfo_assoc_id << '\n';
                }
                if (flag & MSG_NOTIFICATION) {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Notification received from : " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << '\n';
                } else {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << msgLen << "bytes received received from : " << inet_ntoa(addr.sin_addr) << ":"
                              << ntohs(addr.sin_port) << '\n';
                }
            }
            sleep(5);
        }
    }
    int _fd;
    std::thread _receiverThread;
};

#endif //SCTP_POC_COMMON_H
