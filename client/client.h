#ifndef SCTP_POC_CLIENT_H
#define SCTP_POC_CLIENT_H

#include "common.h"

class client {
public:
    client();
    void create_socket() const;
private:
    Socket _sock;
};

#endif //SCTP_POC_CLIENT_H
