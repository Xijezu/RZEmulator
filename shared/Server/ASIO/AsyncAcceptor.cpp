#include "AsyncAcceptor.h"
#include "XSocket.h"

AsyncAcceptor::AsyncAcceptor(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16_t port) : _acceptor(ioContext), _endpoint(NGemity::Net::make_address(bindIp), port),
                                                                                                              _socket(ioContext), _closed(false), _socketFactory(std::bind(&AsyncAcceptor::DefeaultSocketFactory, this))
{
}

void AsyncAcceptor::AsyncAccept()
{
    _acceptor.async_accept(_socket, [this](boost::system::error_code error) {
        if (!error)
        {
            try
            {
                // this-> is required here to fix an segmentation fault in gcc 4.7.2 - reason is lambdas in a templated class
                std::make_shared<XSocket>(std::move(this->_socket))->Start();
            }
            catch (boost::system::system_error const &err)
            {
                NG_LOG_INFO("network", "Failed to retrieve client's remote address %s", err.what());
            }
        }

        // lets slap some more this-> on this so we can fix this bug with gcc 4.7.2 throwing internals in yo face
        if (!_closed)
            this->AsyncAccept();
    });
}

bool AsyncAcceptor::Bind()
{
    boost::system::error_code errorCode;
    _acceptor.open(_endpoint.protocol(), errorCode);
    if (errorCode)
    {
        NG_LOG_INFO("network", "Failed to open acceptor %s", errorCode.message().c_str());
        return false;
    }

    boost::asio::socket_base::reuse_address option(true);
    _acceptor.set_option(option);

    _acceptor.bind(_endpoint, errorCode);
    if (errorCode)
    {
        NG_LOG_INFO("network", "Could not bind to %s:%u %s", _endpoint.address().to_string().c_str(), _endpoint.port(), errorCode.message().c_str());
        return false;
    }

    _acceptor.listen(NGEMITY_MAX_LISTEN_CONNECTIONS, errorCode);
    if (errorCode)
    {
        NG_LOG_INFO("network", "Failed to start listening on %s:%u %s", _endpoint.address().to_string().c_str(), _endpoint.port(), errorCode.message().c_str());
        return false;
    }

    return true;
}

void AsyncAcceptor::Close()
{
    if (_closed.exchange(true))
        return;

    boost::system::error_code err;
    _acceptor.close(err);
}