#include "server.h"

#include <chrono>
#include <ctime>

server::server() : _sock{-1} {}

void server::create_socket() const {

    std::ofstream ofs{"../out/server_socket.log"};
    if (!ofs) {
        std::cerr << "Server output file could not be created!\n";
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in peerAddr {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {.s_addr = htonl(ADDR)},
    };

    struct sctp_paddrparams heartbeat {
            .spp_hbinterval = 20000,
            .spp_pathmaxrxt = 5,
            .spp_flags = SPP_HB_ENABLE,
    };

    struct sctp_event_subscribe events {
            .sctp_data_io_event = 1,
            .sctp_association_event = 1,
            .sctp_address_event = 1,
            .sctp_send_failure_event = 1,
            .sctp_shutdown_event = 1,
            .sctp_sender_dry_event = 1,
            .sctp_send_failure_event_event = 1,
    };

    //struct sctp_rtoinfo rtoinfo {
    //        //.srto_assoc_id = ,
    //        .srto_initial = 120,
    //        .srto_max = 120,
    //        .srto_min = 800,
    //};

    int sockFd{0};
    if ( (sockFd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) == -1) {
        perror("fails at server socket creation");
        return;
    }
    Socket serverSock{sockFd};

    // Configure heartbeats
    if(setsockopt(serverSock.get_socket_descriptor(), SOL_SCTP, SCTP_PEER_ADDR_PARAMS , &heartbeat, sizeof(heartbeat)) != 0) {
        perror("fails at configuring heartbeats via setsockopt");
        close(serverSock.get_socket_descriptor());
        ofs.close();
        return;
    }

    // Configure events
    if((setsockopt(serverSock.get_socket_descriptor(), SOL_SCTP, SCTP_EVENTS, (void *)&events, sizeof(events)))!= 0) {
        perror("fails at configuring events via setsockopt");
        close(serverSock.get_socket_descriptor());
        ofs.close();
        return;
    }

    // Configure rto
    //if((setsockopt(serverSock.get_socket_descriptor(), SOL_SCTP, SCTP_RTOINFO, &rtoinfo, sizeof(rtoinfo))) != 0) {
    //    perror("fails at configuring rto via setsockopt");
    //    close(serverSock.get_socket_descriptor());
    //    return;
    //}

    // Bind
    if(bind(serverSock.get_socket_descriptor(), (struct sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        perror("fails at binding");
        close(serverSock.get_socket_descriptor());
        ofs.close();
        return;
    }

    auto&& now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << ctime(&now);
    ofs << ctime(&now);

    if (listen(serverSock.get_socket_descriptor(), 10) < 0)
    {
        perror("fails at listening");
        close(serverSock.get_socket_descriptor());
        ofs.close();
        return;
    }

    std::cout << "listening is completed\n";
    ofs << "listening is completed\n";

    std::cout << "[Thread " << std::this_thread::get_id() << "]" << "serverSock.open()" << '\n';
    ofs << "[Thread " << std::this_thread::get_id() << "]" << "serverSock.open()" << '\n';
    serverSock.open();

    while(true) {
        std::cout << "[Thread " << std::this_thread::get_id() << "]" << "server endless loop" << '\n';
        ofs << "[Thread " << std::this_thread::get_id() << "]" << "server endless loop" << '\n';
        serverSock.query();
        ofs.flush();
        sleep(10);
    }
    ofs.close();
}

int main() {
    server{}.create_socket();
}