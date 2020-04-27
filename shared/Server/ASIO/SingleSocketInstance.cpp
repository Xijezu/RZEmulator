#include "SingleSocketInstance.h"

#include "Log.h"
#include "NetworkThread.h"
#include "XSocket.h"

NGemity::SingleSocketInstance::SingleSocketInstance()
    : m_pNetworkThread(nullptr)
{
}

void NGemity::SingleSocketInstance::InitializeSingleSocketInstance()
{
    if (m_pNetworkThread != nullptr)
    {
        NG_LOG_ERROR("network", "SingleSocketInstance: Networkthread already started, initialization failed!");
        return;
    }

    m_pNetworkThread = std::make_unique<XSocketThread>();
    m_pNetworkThread->Start();
}

void NGemity::SingleSocketInstance::AddSocket(std::shared_ptr<XSocket> sock)
{
    if (m_pNetworkThread != nullptr)
        m_pNetworkThread->AddSocket(sock);
}

void NGemity::SingleSocketInstance::StopSingleSocketInstance()
{
    if (m_pNetworkThread != nullptr)
        m_pNetworkThread->Stop();

    m_pNetworkThread.release();
    m_pNetworkThread = nullptr;
}

int32_t NGemity::SingleSocketInstance::GetConnectionCount()
{
    if (m_pNetworkThread != nullptr)
        return m_pNetworkThread->GetConnectionCount();
    return -1;
}