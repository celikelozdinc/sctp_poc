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

    void query() const {
        struct sctp_status status;
        memset(&status, 0, sizeof(struct sctp_status));
        int slen = sizeof(sctp_status);
        if (getsockopt(_fd, SOL_SCTP, SCTP_STATUS, (void *)&status, (socklen_t *)&slen) != 0) {
            std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "Couldn't query current status!\n";
            return;
        }
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "{\n";
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "ASSOC ID = " << status.sstat_assoc_id << '\n';
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "STATE = " << status.sstat_state << '\n';
        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "}\n";
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
            std::cout << "\n\n\t\t[Thread " << _receiverThread.get_id() << "]" << "_on_open()"  << '\n';
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
                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "PPID : " << sri.sinfo_ppid << '\n'; //means client port
                if (sri.sinfo_assoc_id == 0) {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]" << "Assoc id = 0" << '\n';
                } else {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Assoc id => " << sri.sinfo_assoc_id << '\n';
                }
                if (flag & MSG_NOTIFICATION) {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Notification received from : " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << '\n';
                    const auto& notification = static_cast<sctp_notification*>((void*)inputBuffer);
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Notification Type : " << notification->sn_header.sn_type << '\n';
                    if (SCTP_ASSOC_CHANGE == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_ASSOC_CHANGE notification has been received.\n";
                        const auto& sac = static_cast<sctp_assoc_change*>(&(notification->sn_assoc_change));
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Assoc id => " << sac->sac_assoc_id << '\n';
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Notification type = " << sac->sac_state << '\n';
                        switch(sac->sac_state) {
                            break; case SCTP_COMM_UP : {
                                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_COMM_UP\n";
                                //invoke callback func
                            }
                            break; case SCTP_COMM_LOST : {
                                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_COMM_LOST\n";
                                //invoke callback func
                            }
                            break; case SCTP_RESTART : {
                                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_RESTART\n";
                                //invoke callback func
                            }
                            break; case SCTP_SHUTDOWN_COMP : {
                                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_SHUTDOWN_COMP\n";
                                //invoke callback func
                            }
                            break; case SCTP_CANT_STR_ASSOC : {
                                std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_CANT_STR_ASSOC\n";
                                //invoke callback func
                            }
                        }
                    } else if (SCTP_DATA_IO_EVENT == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_DATA_IO_EVENT notification has been received.\n";
                    } else if (SCTP_SHUTDOWN_EVENT == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_SHUTDOWN_EVENT notification has been received.\n";
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Closing socket...\n";
                        //invoke callback func
                        close(_fd);
                        return;
                    } else if (SCTP_SENDER_DRY_EVENT == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_SENDER_DRY_EVENT notification has been received.\n";
                    } else if (SCTP_SEND_FAILED_EVENT == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_SEND_FAILED_EVENT notification has been received.\n";
                    } else if (SCTP_ADAPTATION_INDICATION == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_ADAPTATION_INDICATION notification has been received.\n";
                    } else if (SCTP_AUTHENTICATION_EVENT == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_AUTHENTICATION_EVENT notification has been received.\n";
                    } else if (SCTP_PEER_ADDR_CHANGE == notification->sn_header.sn_type) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "SCTP_PEER_ADDR_CHANGE notification has been received.\n";
                        const auto& sac = static_cast<sctp_paddr_change*>(&(notification->sn_paddr_change));
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Assoc id => " << sac->spc_assoc_id << '\n';
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Notification type = " << sac->spc_type << '\n';

                    } else {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "Other notification type\n";
                    }
                } else {
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << msgLen << " bytes received received from : " << inet_ntoa(addr.sin_addr) << ":"
                              << ntohs(addr.sin_port) << '\n';

                    char* dataRef = static_cast<char*>(inputBuffer);
                    std::string received{dataRef, inputBufferSize};
                    std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << received << " received received from : " << inet_ntoa(addr.sin_addr) << ":"
                              << ntohs(addr.sin_port) << '\n';
                    //invoke callback func
                    char buf[1024];
                    memset(buf, 0, sizeof(buf));
                    if ('-' == received[0]) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "This message is received from sctp client.\n";
                        snprintf(buf, sizeof(buf)-1, "HELLO FROM SERVER---");
                        sctp_sendmsg(_fd, buf, 1024, (struct sockaddr*)&addr, sizeof(addr), htonl(ADDR), 0, 0, 0, 0);
                    } else if ('H' == received[0]) {
                        std::cout << "\t\t[Thread " << _receiverThread.get_id() << "]"  << "This message is received from sctp server.\n";
                        snprintf(buf, sizeof(buf)-1, "---HELLO FROM CLIENT");
                        sctp_sendmsg(_fd, buf, 1024, (struct sockaddr*)&addr, sizeof(addr), htonl(ADDR), 0, 0, 0, 0);
                    } else {
                        std::cerr << "\t\t[Thread " << _receiverThread.get_id() << "]"  << " UNEXPECTED MESSAGE IS RECEIVED!!!\n";
                    }
                }
            }
            sleep(1);
        }
    }
    int _fd;
    std::thread _receiverThread;
};

#endif //SCTP_POC_COMMON_H
