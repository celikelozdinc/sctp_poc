#include "client.h"

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <unistd.h>


client::client() : _sock{-1} {}

void client::create_socket() const {
    std::ofstream ofs{"../out/client_socket.log"};
    if (!ofs) {
        std::cerr << "Client output file could not be created!\n";
        exit(EXIT_FAILURE);
    }


    // Peer Address
    struct sockaddr_in peerAddr {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {.s_addr = htonl(ADDR)},
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


    struct sctp_initmsg initMsg {
        .sinit_num_ostreams = 5,
        .sinit_max_instreams = 5,
        .sinit_max_attempts = 7,
        .sinit_max_init_timeo = 1000,
    };

    struct sctp_paddrparams heartbeat {
        .spp_hbinterval = 20000,
        .spp_pathmaxrxt = 5,
        .spp_flags = SPP_HB_ENABLE,
    };

    //struct sctp_rtoinfo rtoinfo {
    //        //.srto_assoc_id = ,
    //        .srto_initial = 120,
    //        .srto_max = 120,
    //        .srto_min = 800,
    //};


    int sockFd{0};
    if ( (sockFd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) == -1) {
        perror("fails at client socket creation");
        return;
    }

    Socket clientSock{sockFd};

    // Configure heartbeats
    if (!clientSock.set_socket_option(SCTP_PEER_ADDR_PARAMS, &heartbeat)) {
        perror("fails at configuring heartbeats via setsockopt");
        ofs.close();
    }

    // Configure events
    if (!clientSock.set_socket_option(SCTP_EVENTS, &events)) {
        perror("fails at configuring events via setsockopt");
        ofs.close();
    }

    // Configure init message
    if (!clientSock.set_socket_option(SCTP_INITMSG, &initMsg)) {
        perror("fails at configuring init message via setsockopt");
        ofs.close();
    }

    // Configure rto
    //if((setsockopt(clientSock.get_socket_descriptor(), SOL_SCTP, SCTP_RTOINFO, &rtoinfo, sizeof(rtoinfo))) != 0) {
    //    perror("fails at configuring rto via setsockopt");
    //    close(clientSock.get_socket_descriptor());
    //    return;
    //}

    // connect
    if((sctp_connectx(clientSock.get_socket_descriptor(), (struct sockaddr*)&peerAddr, 1, nullptr)) < 0)
    {
        perror("fails at connect");
        ofs.close();
        return;
    }

    std::cout << "CONNECTED!\n";
    ofs << "CONNECTED!\n";

    std::cout << "[Thread " << std::this_thread::get_id() << "]" << "clientSock.open()" << '\n';
    ofs << "[Thread " << std::this_thread::get_id() << "]" << "clientSock.open()" << '\n';
    clientSock.open();

    getchar(); // waits for user input

    char initialMsg[BUFSIZE];
    memset(initialMsg, 0, sizeof(initialMsg));
    snprintf(initialMsg, sizeof(initialMsg)-1, "-----HELLO FROM CLIENT");
    int msgLen{0};
    msgLen = clientSock.send(initialMsg, BUFSIZE, (struct sockaddr*)&peerAddr);
    if (-1 == msgLen)
    {
        std::cout << "\t[Thread " << std::this_thread::get_id() << "]" << "send() failed from client socket\n";
        perror("send() failed from client socket\n");
        return;
    }
    std::cout << "\t[Thread " << std::this_thread::get_id() << "]" << "initial message containing " << msgLen << " bytes sent to peer!\n";
    ofs << "\t[Thread " << std::this_thread::get_id() << "]" << "initial message containing " << msgLen << " bytes sent to peer!\n";

    while(true) {
        std::cout << "\t[Thread " << std::this_thread::get_id() << "]" << "client endless loop" << '\n';
        ofs << "\t[Thread " << std::this_thread::get_id() << "]" << "client endless loop" << '\n';
        clientSock.query();
        ofs.flush();
        sleep(10);
    }
    ofs.close();
}

int main() {
    client{}.create_socket();
}