#ifndef SCTP_POC_SERVER_H
#define SCTP_POC_SERVER_H

#include "common.h"

class server {
public:
    server();
    void create_socket() const;
private:
    Socket _sock;
};


#endif //SCTP_POC_SERVER_H
