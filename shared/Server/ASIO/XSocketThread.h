//
// Created by xijezu on 7/20/18.
//
#ifndef NGEMITY_XSOCKETTHREAD_H
#define NGEMITY_XSOCKETTHREAD_H

#include "XSocket.h"

template<class T>
class XSocketThread : public NetworkThread<XSocket>
{
    public:
        void SocketAdded(std::shared_ptr<XSocket> sock) override
        {
            sock->SetSession(new T{sock.get()});
            sock->Start();
        }

        void SocketRemoved(std::shared_ptr<XSocket> sock) override
        {

        }
};

#endif //NGEMITY_XSOCKETTHREAD_H
